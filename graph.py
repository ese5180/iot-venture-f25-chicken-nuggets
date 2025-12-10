import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.gridspec import GridSpec
import matplotlib
from collections import deque
import re
import time

# -----------------------------
# Configuration
# -----------------------------
SERIAL_PORT = "/dev/tty.usbmodem0010507421083" 
BAUD_RATE = 115200
MAX_POINTS = 1000              # Increased to ensure buffer holds enough data for the time window
ROLLING_WINDOW_SIZE = 5        # Size of the averaging window
TIME_WINDOW = 20               # <--- Fixed time width of the graph (in seconds)
YMAX = 5
# -----------------------------

# Regex for parsing
NO_HUMANS_PATTERN = re.compile(r"no_humans_present=(\d+)")

# Serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# Buffer for plotting: stores tuples (time_elapsed, rounded_value)
data_buffer = deque(maxlen=MAX_POINTS)

# Buffer for math: stores the last N raw values
rolling_window = deque(maxlen=ROLLING_WINDOW_SIZE)

# Capture start time
start_time = time.time()

def parse_no_humans_present(line):
    """Extract integer 0–5 from STHS34PF80 output."""
    match = NO_HUMANS_PATTERN.search(line)
    if match:
        return int(match.group(1))
    return None

# -----------------------------
# DASHBOARD LAYOUT
# -----------------------------
plt.style.use("seaborn-v0_8-darkgrid")

fig = plt.figure(figsize=(10, 6))
gs = GridSpec(4, 1, height_ratios=[1, 3, 0.1, 0.3], figure=fig)

# --- TOP READOUT DISPLAY ---
ax_readout = fig.add_subplot(gs[0, 0])
ax_readout.axis("off")
readout_text = ax_readout.text(
    0.5, 0.5, "Number of Occupants: --",
    ha="center", va="center",
    fontsize=28, fontweight="bold"
)

# --- MAIN GRAPH ---
ax = fig.add_subplot(gs[1, 0])
ax.set_title(f"Occupants (Rolling Avg: {ROLLING_WINDOW_SIZE}, Window: {TIME_WINDOW}s)")
ax.set_xlabel("Time Elapsed (s)")
ax.set_ylabel("Count")
ax.set_ylim(0, YMAX)

# Force the Y-axis to only show integer ticks
ax.yaxis.set_major_locator(matplotlib.ticker.MaxNLocator(integer=True))

line, = ax.plot([], [], lw=2)
fill_poly = None

def update(frame):
    global fill_poly

    # Read & parse serial line
    try:
        raw = ser.readline().decode(errors='ignore').strip()
        value = parse_no_humans_present(raw)
    except:
        value = None

    # Update logic
    if value is not None:
        # 1. Add raw value to the rolling window
        rolling_window.append(value)
        
        # 2. Calculate average AND ROUND to nearest integer
        avg_value = int(round(sum(rolling_window) / len(rolling_window)))

        # 3. Calculate elapsed time
        current_time = time.time() - start_time
        
        # 4. Add (time, rounded_value) to the main plot buffer
        data_buffer.append((current_time, avg_value))

        # Update top readout
        readout_text.set_text(f"Number of Occupants: {avg_value}")

    # If we have data, unpack it for plotting
    if len(data_buffer) > 0:
        x = [d[0] for d in data_buffer]
        y = [d[1] for d in data_buffer]
        
        line.set_data(x, y)

        if fill_poly is not None:
            fill_poly.remove()

        fill_poly = ax.fill_between(x, y, alpha=0.25, color="tab:blue")

        # --- FIXED ROLLING WINDOW LOGIC ---
        last_time = x[-1]
        
        if last_time < TIME_WINDOW:
            # If we haven't reached the window size yet, fix 0 to TIME_WINDOW
            ax.set_xlim(0, TIME_WINDOW)
        else:
            # Otherwise, slide the window (Current Time - Window, Current Time)
            ax.set_xlim(last_time - TIME_WINDOW, last_time)

    return line, fill_poly, readout_text

ani = animation.FuncAnimation(fig, update, interval=50, blit=False)

plt.tight_layout()
plt.show()