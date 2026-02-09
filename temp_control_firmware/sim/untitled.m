%% log_from_serial.m
% Logging UART telemetry lines like:
% {"T_meas":46.63,"T_ref":35.00,"PWM":0.0}
%
% Saves CSV (raw contains clean JSON only)
%
% Author: Borys Ovsiyenko

clear; clc;

%% User settings
port = "COM4";
baud = 115200;
duration_s = 30;
out_csv = "telemetry_log.csv";

enable_live_plot = true;
%% Open serial
s = serialport(port, baud, "Timeout", 1.0);
configureTerminator(s, "LF");
flush(s);

fprintf("Logging from %s @ %d baud for %ds...\n", port, baud, duration_s);

%% Storage
t0 = tic;
ts = [];
T_meas = [];
T_ref  = [];
PWM    = [];
rawJson = strings(0,1);   % will store JSON only (no CRC)

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

%% Main loop
n_bad_parse = 0;
n_non_json  = 0;

while toc(t0) < duration_s
    if s.NumBytesAvailable == 0
        pause(0.01);
        continue;
    end

    line = strtrim(readline(s));
    if line == ""
        continue;
    end

    % If some old firmware still sends "*CRC", strip it safely:
    starIdx = strfind(line, "*");
    if ~isempty(starIdx)
        line = extractBefore(line, starIdx(1));  % keep only JSON
        line = strtrim(line);
    end

    % We only care about JSON lines
    if ~startsWith(line, "{")
        n_non_json = n_non_json + 1;
        continue;
    end

    % Parse JSON
    try
        obj = jsondecode(char(line));
        t_now = toc(t0);

        % Fields (allow case variations)
        if isfield(obj, "T_meas"); tm = obj.T_meas; else; tm = obj.t_meas; end
        if isfield(obj, "T_ref");  tr = obj.T_ref;  else; tr = obj.t_ref;  end
        if isfield(obj, "PWM");    pw = obj.PWM;    else; pw = obj.pwm;    end

        ts(end+1,1)     = t_now;
        T_meas(end+1,1) = double(tm);
        T_ref(end+1,1)  = double(tr);
        PWM(end+1,1)    = double(pw);
        rawJson(end+1,1)= string(line);  % save clean JSON only

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

%% Save results
data = table(ts, T_ref, T_meas, PWM, rawJson, ...
    'VariableNames', {'t_s','T_ref_C','T_meas_C','PWM_percent','json'});

writetable(data, out_csv);
fprintf("Saved %d samples to %s\n", height(data), out_csv);
fprintf("Skipped: non-json=%d, bad parse=%d\n", n_non_json, n_bad_parse);

% Cleanup
clear s;
