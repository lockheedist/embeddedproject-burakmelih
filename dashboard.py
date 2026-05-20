import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.gridspec import GridSpec
from collections import deque
import datetime

PORT = 'COM3'
BAUD = 115200
MAX_POINTS = 50

TEMP_THRESHOLD = 30.0
HUM_THRESHOLD  = 70.0
LUX_THRESHOLD  = 10.0

temp_data = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
hum_data  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
lux_data  = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
ax_data   = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
ay_data   = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)
az_data   = deque([0]*MAX_POINTS, maxlen=MAX_POINTS)

fig = plt.figure(figsize=(14, 8), facecolor='#1a1a2e')
fig.canvas.manager.set_window_title('Multi-Sensor Environmental Monitor')

gs = GridSpec(3, 3, figure=fig, hspace=0.45, wspace=0.35)

ax1     = fig.add_subplot(gs[0, 0])
ax2     = fig.add_subplot(gs[0, 1])
ax3     = fig.add_subplot(gs[0, 2])
ax4     = fig.add_subplot(gs[1, :])
ax_info = fig.add_subplot(gs[2, :])

for ax in [ax1, ax2, ax3, ax4, ax_info]:
    ax.set_facecolor('#16213e')
    ax.tick_params(colors='#e0e0e0', labelsize=8)
    for spine in ax.spines.values():
        spine.set_edgecolor('#0f3460')

fig.text(0.5, 0.97, 'Multi-Sensor Environmental Monitor',
         ha='center', va='top', fontsize=16, fontweight='bold',
         color='#e94560', family='monospace')

fig.text(0.5, 0.93, 'EE0827  |  Burak Keskin & Melih Nuri Buyuk  |  STM32 Nucleo-F401RE',
         ha='center', va='top', fontsize=9, color='#a0a0c0', family='monospace')

try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
    print(f"Port {PORT} is open!")
except Exception as e:
    print(f"Port is not available: {e}")
    exit()

alert_active = False

def update(frame):
    global alert_active
    try:
        line = ser.readline().decode('utf-8').strip()
        print(f"Incoming: {line}")
        if 'Temp:' not in line:
            return

        parts = line.split()
        temp = float(parts[0].split(':')[1].replace('C',''))
        hum  = float(parts[1].split(':')[1].replace('%',''))
        lux  = float(parts[2].split(':')[1])
        acx  = float(parts[3].split(':')[1])
        acy  = float(parts[4].split(':')[1])
        acz  = float(parts[5].split(':')[1])

        temp_data.append(temp)
        hum_data.append(hum)
        lux_data.append(lux)
        ax_data.append(acx)
        ay_data.append(acy)
        az_data.append(acz)

        alert_active = (temp > TEMP_THRESHOLD or
                        hum  > HUM_THRESHOLD  or
                        lux  < LUX_THRESHOLD)

        # SICAKLIK
        ax1.clear()
        ax1.set_facecolor('#16213e')
        ax1.plot(list(temp_data), color='#e94560', linewidth=1.5)
        ax1.fill_between(range(MAX_POINTS), list(temp_data), alpha=0.2, color='#e94560')
        ax1.axhline(y=TEMP_THRESHOLD, color='#ff6b6b', linestyle='--', linewidth=0.8, alpha=0.7, label=f'Threshold {TEMP_THRESHOLD}C')
        ax1.set_title(f'Temperature: {temp:.1f}C', color='#e94560', fontsize=10, fontweight='bold', pad=4)
        ax1.set_ylim(0, 50)
        ax1.legend(fontsize=7, facecolor='#1a1a2e', labelcolor='white', edgecolor='#0f3460')
        ax1.tick_params(colors='#e0e0e0', labelsize=7)
        for sp in ax1.spines.values(): sp.set_edgecolor('#0f3460')

        # NEM
        ax2.clear()
        ax2.set_facecolor('#16213e')
        ax2.plot(list(hum_data), color='#00b4d8', linewidth=1.5)
        ax2.fill_between(range(MAX_POINTS), list(hum_data), alpha=0.2, color='#00b4d8')
        ax2.axhline(y=HUM_THRESHOLD, color='#90e0ef', linestyle='--', linewidth=0.8, alpha=0.7, label=f'Threshold {HUM_THRESHOLD}%')
        ax2.set_title(f'Humidity: {hum:.1f}%', color='#00b4d8', fontsize=10, fontweight='bold', pad=4)
        ax2.set_ylim(0, 100)
        ax2.legend(fontsize=7, facecolor='#1a1a2e', labelcolor='white', edgecolor='#0f3460')
        ax2.tick_params(colors='#e0e0e0', labelsize=7)
        for sp in ax2.spines.values(): sp.set_edgecolor('#0f3460')

        # LUX
        ax3.clear()
        ax3.set_facecolor('#16213e')
        ax3.plot(list(lux_data), color='#ffd60a', linewidth=1.5)
        ax3.fill_between(range(MAX_POINTS), list(lux_data), alpha=0.2, color='#ffd60a')
        ax3.set_title(f'Lux: {lux:.1f} Lux', color='#ffd60a', fontsize=10, fontweight='bold', pad=4)
        ax3.set_ylim(0, 1000)
        ax3.tick_params(colors='#e0e0e0', labelsize=7)
        for sp in ax3.spines.values(): sp.set_edgecolor('#0f3460')

        # IVMEOLCER
        ax4.clear()
        ax4.set_facecolor('#16213e')
        ax4.plot(list(ax_data), color='#e94560', linewidth=1.2, label='Ax')
        ax4.plot(list(ay_data), color='#00b4d8', linewidth=1.2, label='Ay')
        ax4.plot(list(az_data), color='#80ed99', linewidth=1.2, label='Az')
        ax4.axhline(y=0, color='#ffffff', linewidth=0.5, alpha=0.3)
        ax4.set_title('MPU6050 - Accelerometer (g)', color='#a0a0c0', fontsize=10, fontweight='bold', pad=4)
        ax4.set_ylim(-2, 2)
        ax4.legend(loc='upper right', fontsize=8, facecolor='#1a1a2e', labelcolor='white', edgecolor='#0f3460')
        ax4.tick_params(colors='#e0e0e0', labelsize=7)
        for sp in ax4.spines.values(): sp.set_edgecolor('#0f3460')

        # STATUS BAR
        ax_info.clear()
        ax_info.set_facecolor('#3d0000' if alert_active else '#0f3460')
        ax_info.axis('off')
        status_text = '!  ALARM ACTIVE  !' if alert_active else 'SYSTEM NORMAL'
        status_color = '#e94560' if alert_active else '#80ed99'
        ax_info.text(0.02, 0.5, status_text, transform=ax_info.transAxes,
                     fontsize=12, fontweight='bold', color=status_color, va='center')
        now = datetime.datetime.now().strftime('%H:%M:%S')
        ax_info.text(0.98, 0.5,
                     f'Last Update: {now}   |   Thresholds: TEMP>{TEMP_THRESHOLD}C  HUM>{HUM_THRESHOLD}%  LUX<{LUX_THRESHOLD}Lux',
                     transform=ax_info.transAxes, fontsize=8,
                     color='#a0a0c0', va='center', ha='right')

    except Exception as e:
        print(f"Error: {e}")

ani = animation.FuncAnimation(fig, update, interval=100,
                               cache_frame_data=False, save_count=100)
plt.tight_layout(rect=[0, 0, 1, 0.92])
plt.show()
ser.close()
