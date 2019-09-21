# Contains code for a perceptron classifier that, contrary to the traditional 
#   implementation, performs classification through linear regression
#
# The code creates the linear regression perceptron classifier, and runs it on 
#   the given data, performing cross-fold validation and determining the final 
#   weights and accuracies.
#
# Linear regression perceptron classifier programmed by Ryan Neisess

import math
import numpy
import collections
from sklearn.metrics import accuracy_score
from random import randint
from statistics import mean
from sklearn.metrics import confusion_matrix

def classify(a, class_labels, medians):
	label = -1
	num_meds = len(medians)
	i = 0
	for i in range(0, num_meds):
		if a < medians[i]:
			return class_labels[i]
	return class_labels[-1]

def LRPC(data, num_folds, tol, max_iter, class_labels, skips):
	feats = []
	truths = []
	for entry in data:
		feats.append(entry[:-1])
		truths.append(entry[-1])

	## Normalize the data relative to the range of the class labels

	## First find maxima and minima for each col/feature
	#num_cols = len(feats[0])
	#num_rows = len(feats)
	#maxima = []
	#minima = []
	#for col in range(0, num_cols):
	#	max = min = feats[0][col]
	#	for row in range(0, num_rows):
	#		if feats[row][col] > max:
	#			max = feats[row][col]
	#		if feats[row][col] < min:
	#			min = feats[row][col]
	#	maxima.append(max)
	#	minima.append(min)

	#class_max = class_min = class_labels[0]
	#for label in class_labels:
	#	if label > class_max:
	#		class_max = label
	#	if label < class_min:
	#		class_min = label

	## Now normalize the data relative to the range of the class labels
	#for col in range(0, num_cols):
	#	for row in range(0, num_rows):
	#		feats[row][col] = ((feats[row][col] - minima[col]) / (maxima[col] - minima[col])) * (class_max - class_min) + class_min

	cv_accs = []
	matrices= []
	for fold in range(0, num_folds):
		train_feats = feats[fold + 1::num_folds] + feats[fold + 2::num_folds]
		train_truths = truths[fold + 1::num_folds] + truths[fold + 2::num_folds]
		# use cv_feats[i::num_folds], cv_truths[i::num_folds] for testing

		test_feats = feats[fold::num_folds]
		test_truths = truths[fold::num_folds]

		# # # Begin Linear Regression Perceptron Classifier Algorithm

		medians = []
		for i in range(0, len(class_labels) - 1):
			medians.append((class_labels[i] + class_labels[i + 1]) / 2)

		weights = []
		num_feats = len(train_feats[0])
		for i in range(0, num_feats):
			weights.append(0) # Initialize all weights to 0
		bias = 0

		for iter in range(0, max_iter):
			i = 0 # curr row
			max_diff = 0
			for row in train_feats:
				a = 0.0 # activation score, use for lin regr
				for j in range(0, num_feats):
					if j not in skips:
						a += weights[j] * row[j] + bias

				# Now determine the appropriate class label and check
				guess = classify(a, class_labels, medians)
				if guess != train_truths[i]: # incorrect guess, update weights
					for j in range(0, num_feats):
						if j not in skips:
							update = (train_truths[i] - guess) * row[j] * 0.70
							max_diff = abs(weights[j] - update)
							weights[j] += update
					if train_truths[i] > guess:
						bias += 1
					else:
						bias -= 1
				i += 1 # move on to next row of data
			if max_diff < tol:
				print("iter reached: %d" % iter)
				break

		# # # End Linear Regresion Perceptron Classifier Algorithm

		# Now check using test data
		guesses = []
		for row in test_feats:
			a = 0.0 # activation score, use for classification
			for j in range(0, num_feats):
				a += weights[j] * row[j] + bias
			# Now determine the appropriate class label and check
			guesses.append(classify(a, class_labels, medians))

		acc = accuracy_score(test_truths, guesses)
		matrices.append(confusion_matrix(test_truths, guesses, labels = class_labels))
		cv_accs.append(acc)

	print("results: ", cv_accs)
	print("avg acc: ", mean(cv_accs))
	cm_acc = cv_accs[0]
	max_i = 0
	for i in range(0, num_folds):
		if cv_accs[i] > cm_acc:
			cm_acc = cv_accs[i]
			max_i = i
	print("matrix of best performance: ")
	print(matrices[max_i])

	return cv_accs, matrices[max_i]

def LRPC_feat_optimize(data, num_folds, tol, max_iter, class_labels, num_skip):
	accs = []
	skips = []
	matrix = []

	max_acc = mean(LRPC(data, num_folds, tol, max_iter, class_labels, skips))

	for i in range(num_skip): # max number of feats to drop
		num_feats = len(data[0]) - 1
		avg_list = []
		for j in range(0, num_feats):
			temp_skips = list(skips)
			temp_skips.append(j)
			print("  - -  temp_skips contents: ", temp_skips)
			results = LRPC(data, num_folds, tol, max_iter, class_labels, temp_skips)
			avg_list.append(mean(results))

		rel_max = avg_list[0]
		max_index = 0
		for j in range(0, num_feats):
			if avg_list[j] > rel_max:
				rel_max = avg_list[j]
				max_index = j
		skips.append(max_index)

		print("max_index: ", max_index, skips)

		curr_acc, matrix = LRPC(data, num_folds, tol, max_iter, class_labels, skips)
		curr_acc = mean(curr_acc)
		if curr_acc < max_acc:
			skips.remove(min_index)
			print("no improvements")
			return max_acc
		else:
			max_acc = curr_acc
	
	print("final skips: ", skips)
	return max_acc, matrix

if __name__ == "__main__":
	data = numpy.loadtxt(fname = "data/reduced_cleveland_hungarian.csv", delimiter = ",")

	data = data.tolist()

	#large_data = []
	#data_len = len(data)
	#for i in range(0, 1728):
	#	large_data.append(data[randint(0, data_len - 1)])

	#results = LRPC_feat_optimize(data, 3, 0.001, 1200, [0, 1, 2, 3, 4], 3)
	results, matrix = LRPC(data, 3, 0.001, 1200, [0, 1, 2, 3, 4], [])
	print("")
	numpy.savetxt("matrices/matrix_c_h.csv", matrix, delimiter = ",", fmt = "%d")

	fdata = numpy.loadtxt(fname = "data/filtered_cleveland.csv", delimiter = ",")
	fdata = fdata.tolist()
	fresults, fmatrix = LRPC(fdata, 3, 0.001, 1200, [0, 1, 2, 3, 4], [])
	print("")
	numpy.savetxt("matrices/matrix_c.csv", fmatrix, delimiter = ",", fmt = "%d")

	l1 = numpy.loadtxt(fname = "data/LinearData1.txt", delimiter = ",")
	l1 = l1.tolist()
	l1r, l1m = LRPC(l1, 3, 0.001, 1200, [0, 1, 2, 3, 4], [])
	print("")
	numpy.savetxt("matrices/matrix_l1.csv", l1m, delimiter = ",", fmt = "%d")

	l2 = numpy.loadtxt(fname = "data/LinearData2.txt", delimiter = ",")
	l2 = l2.tolist()
	l2r, l2m = LRPC(l2, 3, 0.001, 1200, [0, 1, 2, 3, 4], [])
	print("")
	numpy.savetxt("matrices/matrix_l2.csv", l2m, delimiter = ",", fmt = "%d")

	n1 = numpy.loadtxt(fname = "data/LinearDataNoise1.txt", delimiter = ",")
	n1 = n1.tolist()
	n1r, n1m = LRPC(n1, 3, 0.001, 1200, [0, 1, 2, 3, 4], [])
	print("")
	numpy.savetxt("matrices/matrix_ln1.csv", n1m, delimiter = ",", fmt = "%d")

	n2 = numpy.loadtxt(fname = "data/LinearDataNoise2.txt", delimiter = ",")
	n2 = n2.tolist()
	n2r, n2m = LRPC(n2, 3, 0.001, 1200, [0, 1, 2, 3, 4], [])
	print("")
	numpy.savetxt("matrices/matrix_ln2.csv", n2m, delimiter = ",", fmt = "%d")
