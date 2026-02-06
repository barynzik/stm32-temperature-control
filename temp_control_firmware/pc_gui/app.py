import json
import time
import threading
import tkinter as tk
from tkinter import ttk, messagebox

def crc8_0x07(data: bytes) -> int:
    crc = 0
    for x in data:
        crc ^= x
        for _ in range(8):
            crc = ((crc << 1) ^ 0x07) & 0xFF if (crc & 0x80) else (crc << 1) & 0xFF
    return crc


def add_crc_frame(payload: str) -> bytes:
    c = crc8_0x07(payload.encode("ascii"))
    return f"{payload}*{c:02X}\n".encode("ascii")


def strip_and_check_crc(line: str) -> str | None:
    """
    Returns payload if CRC ok, else None.
    Accepts lines like: "<payload>*<2hex>"
    """
    line = line.strip()
    if "*" not in line:
        return None
    payload, crc_hex = line.rsplit("*", 1)
    if len(crc_hex) != 2:
        return None
    try:
        rx = int(crc_hex, 16)
    except ValueError:
        return None
    calc = crc8_0x07(payload.encode("ascii"))
    return payload if calc == rx else None

# Optional serial
try:
    import serial
    import serial.tools.list_ports
except Exception:
    serial = None

# Plot
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure


class TelemetrySource:
    """Abstract telemetry source: either Serial or Demo."""
    def connect(self): raise NotImplementedError
    def disconnect(self): raise NotImplementedError
    def is_connected(self) -> bool: raise NotImplementedError
    def set_setpoint(self, t_ref_c: float): raise NotImplementedError
    def read_telemetry(self) -> dict: raise NotImplementedError


class DemoSource(TelemetrySource):
    """No hardware: generates realistic-ish telemetry."""
    def __init__(self):
        self._connected = False
        self.t_ref = 35.0
        self.t_meas = 25.0
        self.pwm = 0.0
        self._last = time.time()

    def connect(self):
        self._connected = True
        self._last = time.time()

    def disconnect(self):
        self._connected = False

    def is_connected(self) -> bool:
        return self._connected

    def set_setpoint(self, t_ref_c: float):
        self.t_ref = float(t_ref_c)

    def read_telemetry(self) -> dict:
        # Simple first-order thermal response with PI-like behavior (fake)
        now = time.time()
        dt = max(1e-3, now - self._last)
        self._last = now

        # fake controller
        err = self.t_ref - self.t_meas
        self.pwm += 2.0 * err * dt
        self.pwm = max(0.0, min(100.0, self.pwm))

        # fake plant: dT/dt = (K*pwm - (T-ambient))/tau
        ambient = 25.0
        K = 0.45   # degC per %PWM at steady state (rough)
        tau = 25.0 # seconds
        dT = (K * self.pwm - (self.t_meas - ambient)) * (dt / tau)
        self.t_meas += dT

        return {"T_meas": self.t_meas, "T_ref": self.t_ref, "PWM": self.pwm}


class SerialSource(TelemetrySource):
    """
    Protocol (CRC8 poly 0x07):
      Setpoint:           "T35.0*HH\n"
      Telemetry request:  "?*HH\n"
      Response:           '{"T_meas":..,"T_ref":..,"PWM":..}*HH\n'
    """
    def __init__(self, port: str, baud: int = 115200, timeout: float = 0.5):
        if serial is None:
            raise RuntimeError("pyserial is not installed")
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self.ser = None
        self._connected = False

    def connect(self):
        self.ser = serial.Serial(self.port, self.baud, timeout=self.timeout)
        self._connected = True
        # Small flush
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def disconnect(self):
        self._connected = False
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
        self.ser = None

    def is_connected(self) -> bool:
        return self._connected and self.ser is not None and self.ser.is_open

    def set_setpoint(self, t_ref_c: float):
        if not self.is_connected():
            return
        payload = f"T{float(t_ref_c):.1f}"
        self.ser.write(add_crc_frame(payload))

    def _read_valid_payload_line(self, max_lines: int = 5) -> str | None:
        """
        Reads up to max_lines lines and returns a payload string (CRC-checked if framed),
        or None if nothing valid arrived.
        """
        if not self.is_connected():
            return None

        for _ in range(max_lines):
            line = self.ser.readline().decode("ascii", errors="ignore").strip()
            if not line:
                continue

            # Prefer CRC-framed messages
            if "*" in line:
                payload = strip_and_check_crc(line)
                if payload is None:
                    # CRC mismatch or malformed frame -> ignore
                    continue
                return payload

            # Fallback: accept non-CRC payloads (useful if firmware in simple mode)
            return line

        return None

    def read_telemetry(self) -> dict:
        if not self.is_connected():
            return {}

        # request telemetry with CRC
        self.ser.write(add_crc_frame("?"))

        # try a few lines because device might send OK/ERR before JSON
        for _ in range(5):
            payload = self._read_valid_payload_line(max_lines=1)
            if payload is None:
                continue

            # ignore ACK/ERR or other text
            if not payload.startswith("{"):
                continue

            try:
                return json.loads(payload)
            except Exception:
                # malformed JSON -> ignore and keep trying
                continue

        return {}


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("STM32 Temperature Control GUI")
        self.geometry("900x520")

        self.source: TelemetrySource = DemoSource()
        self.poll_hz = 5  # UI update rate
        self._stop = threading.Event()

        # Data for plot
        self.t0 = time.time()
        self.max_points = 600
        self.ts = []
        self.t_meas = []
        self.t_ref = []

        self._build_ui()
        self._refresh_ports()

        self.after(200, self._ui_tick)

    def _build_ui(self):
        # Top frame: connection
        top = ttk.Frame(self, padding=10)
        top.pack(fill="x")

        self.mode_var = tk.StringVar(value="DEMO")
        ttk.Label(top, text="Mode:").pack(side="left")
        ttk.Radiobutton(top, text="Demo", value="DEMO", variable=self.mode_var,
                        command=self._on_mode_change).pack(side="left", padx=5)
        ttk.Radiobutton(top, text="Serial", value="SERIAL", variable=self.mode_var,
                        command=self._on_mode_change).pack(side="left", padx=5)

        ttk.Label(top, text="Port:").pack(side="left", padx=(20, 5))
        self.port_var = tk.StringVar(value="")
        self.port_combo = ttk.Combobox(top, textvariable=self.port_var, width=20, state="readonly")
        self.port_combo.pack(side="left")

        ttk.Button(top, text="Refresh", command=self._refresh_ports).pack(side="left", padx=5)

        self.conn_btn = ttk.Button(top, text="Connect", command=self._toggle_connect)
        self.conn_btn.pack(side="left", padx=10)

        # Setpoint controls
        ttk.Label(top, text="Setpoint (°C):").pack(side="left", padx=(20, 5))
        self.set_var = tk.StringVar(value="35.0")
        self.set_entry = ttk.Entry(top, textvariable=self.set_var, width=8)
        self.set_entry.pack(side="left")
        ttk.Button(top, text="Set", command=self._send_setpoint).pack(side="left", padx=5)

        # Middle: telemetry labels
        mid = ttk.Frame(self, padding=(10, 0, 10, 10))
        mid.pack(fill="x")

        self.lbl_meas = ttk.Label(mid, text="T_meas: -- °C", font=("Segoe UI", 12))
        self.lbl_ref  = ttk.Label(mid, text="T_ref: -- °C",  font=("Segoe UI", 12))
        self.lbl_pwm  = ttk.Label(mid, text="PWM: -- %",     font=("Segoe UI", 12))
        self.lbl_status = ttk.Label(mid, text="Status: disconnected", foreground="gray")

        self.lbl_meas.pack(side="left", padx=10)
        self.lbl_ref.pack(side="left", padx=10)
        self.lbl_pwm.pack(side="left", padx=10)
        self.lbl_status.pack(side="right")

        # Plot area
        fig = Figure(figsize=(7.5, 3.6), dpi=100)
        self.ax = fig.add_subplot(111)
        self.ax.set_title("Temperature vs Time")
        self.ax.set_xlabel("Time [s]")
        self.ax.set_ylabel("Temperature [°C]")
        self.line_meas, = self.ax.plot([], [], label="T_meas")
        self.line_ref,  = self.ax.plot([], [], label="T_ref")
        self.ax.legend()
        self.ax.grid(True)

        self.canvas = FigureCanvasTkAgg(fig, master=self)
        self.canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=(0, 10))

    def _on_mode_change(self):
        # If switching modes while connected, disconnect
        if self.source and self.source.is_connected():
            self.source.disconnect()
        self._stop.set()
        self._stop = threading.Event()

        if self.mode_var.get() == "DEMO":
            self.source = DemoSource()
            self.port_combo.configure(state="disabled")
        else:
            self.port_combo.configure(state="readonly")
        self._update_conn_ui(False)

    def _refresh_ports(self):
        if serial is None:
            self.port_combo["values"] = []
            return
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_combo["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def _toggle_connect(self):
        if self.source.is_connected():
            self.source.disconnect()
            self._update_conn_ui(False)
            return

        try:
            if self.mode_var.get() == "DEMO":
                self.source = DemoSource()
                self.source.connect()
            else:
                port = self.port_var.get().strip()
                if not port:
                    messagebox.showerror("Port", "Select a COM port first.")
                    return
                self.source = SerialSource(port=port, baud=115200)
                self.source.connect()

            self._update_conn_ui(True)
        except Exception as e:
            messagebox.showerror("Connect failed", str(e))
            self._update_conn_ui(False)

    def _update_conn_ui(self, connected: bool):
        self.conn_btn.configure(text="Disconnect" if connected else "Connect")
        self.lbl_status.configure(
            text=("Status: connected" if connected else "Status: disconnected"),
            foreground=("green" if connected else "gray"),
        )

    def _send_setpoint(self):
        try:
            t_ref = float(self.set_var.get())
        except ValueError:
            messagebox.showerror("Setpoint", "Enter a valid number, e.g. 35.0")
            return
        # Optional clamp for your chosen range
        t_ref = max(30.0, min(60.0, t_ref))
        self.set_var.set(f"{t_ref:.1f}")
        if self.source.is_connected():
            self.source.set_setpoint(t_ref)

    def _ui_tick(self):
        # Poll and update UI
        if self.source.is_connected():
            tlm = self.source.read_telemetry()
            if tlm:
                t_meas = float(tlm.get("T_meas", 0.0))
                t_ref  = float(tlm.get("T_ref", 0.0))
                pwm    = float(tlm.get("PWM", 0.0))

                self.lbl_meas.configure(text=f"T_meas: {t_meas:.2f} °C")
                self.lbl_ref.configure(text=f"T_ref: {t_ref:.2f} °C")
                self.lbl_pwm.configure(text=f"PWM: {pwm:.1f} %")

                # Update plot data
                t = time.time() - self.t0
                self.ts.append(t)
                self.t_meas.append(t_meas)
                self.t_ref.append(t_ref)
                if len(self.ts) > self.max_points:
                    self.ts = self.ts[-self.max_points:]
                    self.t_meas = self.t_meas[-self.max_points:]
                    self.t_ref = self.t_ref[-self.max_points:]

                self.line_meas.set_data(self.ts, self.t_meas)
                self.line_ref.set_data(self.ts, self.t_ref)
                self.ax.relim()
                self.ax.autoscale_view()
                self.canvas.draw_idle()

        self.after(int(1000 / self.poll_hz), self._ui_tick)


if __name__ == "__main__":
    app = App()
    app.mainloop()
