import sys
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons

if len(sys.argv) > 1:
	input = sys.argv[1]
else:
	print("Usage : " + sys.argv[0] + " path/to/grades")
	exit(1)
print(input)

# Import grades
file_in = open(input, 'r')
ranges = np.arange(0,30.5,1)
voti = np.empty(0)
for line in file_in:
	tokens = [s for s in line.split('\t') if s]  # remove empty tokens
	if len(tokens) == 10:
		voti = np.append(voti, float(tokens[6]))

# Figure
plt.style.use("ggplot")
fig, ax = plt.subplots()
n, bins, patches = plt.hist(voti, bins=ranges, facecolor='g', alpha=0.75, rwidth=0.5)
plt.subplots_adjust(left=0.25, bottom=0.25)
plt.axis([0, 30.5, 0, 1.5*np.amax(n)])

ax2 = ax.twinx()
ax2.plot(np.cumsum(n)/np.sum(n)*100)
ax2.axes.get_yaxis().set_visible(True)
ax2.grid(False)

# Gui
axcolor = 'lightgoldenrodyellow'
cu = plt.axes([0.25, 0.1, 0.65, 0.03], facecolor=axcolor)
cu_s = Slider(cu, 'Human Cases', 0, 30, valinit=15)
suff = plt.axes([0.25, 0.15, 0.65, 0.03], facecolor=axcolor)
suff_s = Slider(suff, 'Admission', 0, 30, valinit=18)
def update(val):	
	pass
cu_s.on_changed(update)
suff_s.on_changed(update)

plt.show()
