from random import randint

import numpy
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import pylab
from mpl_toolkits.mplot3d import Axes3D

# Calculates the Euclidean distance between two 3D pts
def Euclidean_distance(pt1, pt2):
	i = 0
	sum = 0
	while i < 3:
		sum += (pt1[i] - pt2[i]) ** 2
		i += 1
	return sum ** 0.5

# Calculates the Manhattan distance between two 3D pts
def Manhattan_distance(pt1, pt2):
	i = 0
	sum = 0
	while i < 3:
		sum += abs(pt1[i] - pt2[i])
		i += 1
	return sum

if __name__ == "__main__":
	data = numpy.loadtxt(fname = "3D_spatial_network.txt", delimiter = ",")

	# Find max and min vals for each coord
	long_max = long_min = data[0][0]
	lat_max = lat_min = data[0][1]
	alt_max = alt_min = data[0][2]

	for point in data:
		if point[0] > long_max:
			long_max = point[0]
		elif point[0] < long_min:
			long_min = point[0]

		if point[1] > lat_max:
			lat_max = point[1]
		elif point[1] < lat_min:
			lat_min = point[1]

		if point[2] > alt_max:
			alt_max = point[2]
		elif point[2] < alt_min:
			alt_min = point[2]

	# Normalize data and initialize label arrays
	norm_data = data
	labels = []
	for point in norm_data:
		point[0] = (point[0] - long_min) / (long_max - long_min)
		point[1] = (point[1] - lat_min) / (lat_max - lat_min)
		point[2] = (point[2] - alt_min) / (alt_max - alt_min)
		labels.append(-1)

	# Generate k number of initial means (points)
	k = 4
	i = 0
	means = []
	counts = [] # used to average new means later
	while i < k:
		point = []
		# Append rand nums 0 to 1 with 9 sig figs
		point.append(randint(0, 100000000) / 100000000) 
		point.append(randint(0, 100000000) / 100000000)
		point.append(randint(0, 100000000) / 100000000)
		means.append(point)
		counts.append(1)
		i += 1

	print("    Means: ", means, "\n")

	changes = 0 # keep track of num points that changed labels
	convergence = False

	while convergence != True: # loop until no change
		i = 0
		for point in norm_data:
			j = 0
			curr_distance = 0
			min_distance = 3 # init okay since k >= 1 and 3D data normalized
			min_label = -1
			while j < k:
				# find the min distance between the point and each mean
				curr_distance = Euclidean_distance(point, means[j])
				if curr_distance < min_distance:
					min_distance = curr_distance # save the min distance
					min_label = j # save the associated label
				j += 1
			if labels[i] != min_label:
				labels[i] = min_label # assign shortest distance label
				changes += 1

			i += 1
		# end for point in norm_data

		# choose new means
		i = 0
		while i < k: # reset the counts of each label
			counts[i] = 0
			i += 1
		# why didn't this work instead of the above?
		#for ct in counts: # reset the counts of each label
		#	ct = 0

		for pt_label in labels:
			counts[pt_label] += 1

		print("Counts: ", counts, "\n")

		for point in means:
			point[0] = 0
			point[1] = 0
			point[2] = 0

		i = 0
		for point in norm_data:
			# add the x/y/z coord of point to the x/y/z coord of the apropos label
			means[labels[i]][0] += point[0]
			means[labels[i]][1] += point[1]
			means[labels[i]][2] += point[2]
			i += 1

		# recall: there are k items in counts
		i = 0
		while i < k:
			if counts[i] != 0:
				means[i][0] /= counts[i]
				means[i][1] /= counts[i]
				means[i][2] /= counts[i]
			i += 1

		print("    New means: ", means, "\n")

		# check for convergence
		print("    Changes: ", changes, "\n")
		print(" - - - - - - - - - - - - \n")
		if changes == 0:
			convergence = True
		changes = 0
	# end while !convergence

	#num_clusters = 4 # use k instead
	fig1 = plt.figure(1, figsize=(4,3))
	ax = Axes3D(fig1, rect=[0, 0, 3, 3])
	# use norm_data instead
	#X = [[9.3498486,56.7408757,17.0527715677876],
	#	[9.3501884,56.7406785,17.614840244389],
	#	[8.5856624,57.0106364,32.0776406065856],
	#	[8.5851822,57.0099725,28.7124475240045],
	#	[10.0015925,56.6369145,75.2648335602078],
	#	[10.0019142,56.6373512,71.556921301853],
	#	[9.5685446,57.0522153,1.94681426140926],
	#	[9.5693017,57.05361,2.24977425104904],
	#	[9.912351,57.0257797,3.66317365511644],
	#	[10.1787039,56.592082,33.5740971625897],
	#	[10.1782748,56.592342,33.8217791473094],
	#	[10.1779273,56.5923715,33.0209795956494]]
	#labels = [0, 0, 1, 1, 2, 2, 3, 3, 3, 2, 2, 2] # use my own labels
	ax.scatter([row[0] for row in norm_data], [row[1] for row in norm_data], [row[2] for row in norm_data], \
			   c=[int(i % k) for i in labels], cmap=pylab.cm.gist_ncar)
	ax.w_xaxis.set_ticklabels([])
	ax.w_yaxis.set_ticklabels([])
	ax.w_zaxis.set_ticklabels([])
	ax.set_xlabel('Longitude')
	ax.set_ylabel('Latitude')
	ax.set_zlabel('Altitude')
	ax.dist = 1
