%% log_from_serial.m
% Logging UART telemetry lines like:
% {"T_meas":46.63,"T_ref":35.00,"PWM":0.0}*6D
%
% Saves CSV 

% Author: Borys Ovsiyenko
clear; clc;

%%
port = "COM4";        
baud = 115200;     
duration_s = 30;      
out_csv = "telemetry_log.csv";

enable_crc_check = true;  % set false if you want to accept lines without CRC
enable_live_plot = true;  % set false if you don't want plots

%% ===== Open serial =====
s = serialport(port, baud, "Timeout", 1.0);
configureTerminator(s, "LF");     
flush(s);

fprintf("Logging from %s @ %d baud for %ds...\n", port, baud, duration_s);

%% ===== Storage =====
t0 = tic;
ts = [];          % timestamps [s]
T_meas = [];
T_ref  = [];
PWM    = [];
rawLine = strings(0,1);

% Live plot setup
if enable_live_plot
    f1 = figure; ax1 = axes(f1);
    h_meas = plot(ax1, NaN, NaN); hold(ax1, "on");
    h_ref  = plot(ax1, NaN, NaN);
    grid(ax1, "on");
    xlabel(ax1, "t [s]"); ylabel(ax1, "Temp [C]");
    legend(ax1, "T\_meas", "T\_ref");

    f2 = figure; ax2 = axes(f2);
    h_pwm = plot(ax2, NaN, NaN);
    grid(ax2, "on");
    xlabel(ax2, "t [s]"); ylabel(ax2, "PWM [%]");
end

%% ===== Main loop =====
n_bad_crc = 0;
n_bad_parse = 0;

while toc(t0) < duration_s
    if s.NumBytesAvailable == 0
        pause(0.01);
        continue;
    end

    line = strtrim(readline(s));
    if line == ""
        continue;
    end

    % Expect either JSON*HH or plain JSON
    jsonText = line;
    crc_ok = true;

    starIdx = strfind(line, "*");
    if ~isempty(starIdx)
        jsonText = extractBefore(line, starIdx(1));
        crcText  = extractAfter(line,  starIdx(1));  % e.g. "6D"
        crcText  = upper(strtrim(crcText));

        % Validate CRC text
        if strlength(crcText) >= 2
            crcText = extractBetween(crcText, 1, 2);
        end

        if enable_crc_check
            try
                crc_rx = hex2dec(char(crcText));
                crc_calc = crc8_poly07(uint8(char(jsonText)));
                crc_ok = (crc_rx == crc_calc);
            catch
                crc_ok = false;
            end
        end
    else
        % No '*' in line
        if enable_crc_check
            crc_ok = false; % strict mode
        end
    end

    if ~crc_ok
        n_bad_crc = n_bad_crc + 1;
        continue;
    end

    % Parse JSON
    try
        obj = jsondecode(char(jsonText));
        t_now = toc(t0);

        % Some firmwares use "PWM" or "pwm" etc. Adjust if needed:
        if isfield(obj, "T_meas"); tm = obj.T_meas; else; tm = obj.t_meas; end
        if isfield(obj, "T_ref");  tr = obj.T_ref;  else; tr = obj.t_ref;  end
        if isfield(obj, "PWM");    pw = obj.PWM;    else; pw = obj.pwm;    end

        ts(end+1,1)    = t_now;
        T_meas(end+1,1)= double(tm);
        T_ref(end+1,1) = double(tr);
        PWM(end+1,1)   = double(pw);
        rawLine(end+1,1)= string(line);

        % Live plot update
        if enable_live_plot && mod(numel(ts), 5) == 0
            if isvalid(h_meas)
                set(h_meas, "XData", ts, "YData", T_meas);
                set(h_ref,  "XData", ts, "YData", T_ref);
            end
            if isvalid(h_pwm)
            set(h_pwm, "XData", ts, "YData", PWM);
            end
            drawnow limitrate;
        end

    catch
        n_bad_parse = n_bad_parse + 1;
        continue;
    end
end

%% ===== Save results =====
% Make table/timetable
data = table(ts, T_ref, T_meas, PWM, rawLine, ...
    'VariableNames', {'t_s','T_ref_C','T_meas_C','PWM_percent','raw'});

writetable(data, out_csv);
fprintf("Saved %d samples to %s\n", height(data), out_csv);
fprintf("Skipped: bad CRC=%d, bad parse=%d\n", n_bad_crc, n_bad_parse);

% Cleanup
clear s;

%% ===== CRC-8 (poly 0x07, init 0x00, MSB-first) =====
function crc = crc8_poly07(bytes)
    % bytes: uint8 row/col vector
    poly = uint8(hex2dec('07'));
    crc  = uint8(0);

    for i = 1:numel(bytes)
        crc = bitxor(crc, bytes(i));
        for b = 1:8
            if bitand(crc, uint8(128)) ~= 0
                crc = bitxor(bitshift(crc, 1), poly);
            else
                crc = bitshift(crc, 1);
            end
        end
    end
end
