# This function defines the control logic for Team 1:

from __future__ import division
import math

class Controller:

	def __init__(self): # This initializes all the global variables
		self.hold_strat = 1 # Intially, we will 'hold' the first strategy
		self.curr_strat = 1 # First strategy is a global search
		self.x_last = 0 # This is initialized with vehicle's initial location
		self.y_last = 0 # This is initialized with vehicle's initial location
		self.min_signal = -75 # Minimum signal strength needed to decide if a signal is "found"
		#self.min_signal = -90 # Minimum signal strength needed to decide if a signal is "found"
		self.max_signal = -65 # Minimum signal strength needed to decide if a signal is "found"
		self.min_signal_error = 1 # Minimum signal strength needed to decide if a signal is "found"
		self.s_last = self.min_signal # Initialize "last" signal with a minimum value
		self.signal_r = 10 # Approx Signal Radius (m) - Needs to be an integer
		self.area_h = 10 # Search Area Height (m)
		self.area_w = 20 # Search Area Width (m)

		self.global_search_pt_list = self.define_global_pts()
		#self.circle_search_r0 = self.signal_r/4
		self.circle_search_r0 = 1

		self.circle_start_pt = (0, 0)

		self.best_known_signal = self.min_signal
		self.best_known_loc = (0, 0)

		self.local_best_signal = self.min_signal
		self.local_best_loc = (0, 0)


	##### Global Search - Define Pts - Function: #####
	# This function defines the control pts for the global search function.
	def define_global_pts(self):

		triangle_w = self.signal_r - 1

		x_init_temp = triangle_w/2
		y_init_temp = self.area_h

		pt_list_temp = [(x_init_temp, y_init_temp)]
		temp_flag1 = 1
		temp_flag2 = x_init_temp
		temp_flag3 = 0

		while triangle_w >= 2:

			while temp_flag3 == 0:
				if temp_flag1 == 1:
					x_init_temp = x_init_temp + triangle_w/2
				else:
					x_init_temp = x_init_temp - triangle_w/2

				if y_init_temp == 0:
					y_init_temp = self.area_h
				else:
					y_init_temp = 0

				pt_temp = (x_init_temp, y_init_temp)
				pt_list_temp.append(pt_temp)

				temp_flag2 = temp_flag2 + triangle_w/2

				if ( temp_flag2 + triangle_w/2 ) > self.area_w:
					if temp_flag1 == 1:
						x_init_temp = self.area_w
					else:
						x_init_temp = 0

					if y_init_temp == 0:
						y_init_temp = self.area_h
					else:
						y_init_temp = 0

					pt_temp = (x_init_temp, y_init_temp)
					pt_list_temp.append(pt_temp)

					if temp_flag1== 0:
						temp_flag1 = 1
					else:
						temp_flag1 = 0

					temp_flag2 = 0

					temp_flag3 = 1

			triangle_w = triangle_w - 1
			temp_flag3 = 0

		global_search_pt_list_temp = pt_list_temp

		#print pt_list_temp

		return global_search_pt_list_temp


	##### Circle Search - Define Pts - Function: #####
	# This function defines the control pts for the circle search function.
	def define_circle_pts(self, circle_start_pt_temp, x_last_temp, y_last_temp, x_curr, y_curr, temp_flag1):

		if (x_curr - x_last_temp) == 0:
			angle_temp = math.pi/2
		else:
			angle_temp = math.atan((y_curr - y_last_temp)/(x_curr - x_last_temp))

		angle_temp11 = math.fabs(angle_temp)
		angle_temp1 = math.pi/2 - angle_temp11

		angle_div = 6
		if (y_curr - y_last_temp) >= 0 and (x_curr - x_last_temp) >= 0:
			ang_start = math.pi/2 + angle_temp11
			ang_end = - (math.pi/2 - angle_temp11)
			circle_increm = - math.pi/angle_div
		elif (y_curr - y_last_temp) < 0 and (x_curr - x_last_temp) >= 0:
			ang_start = math.pi/2 - angle_temp11
			ang_end = - (math.pi/2 + angle_temp11)
			circle_increm = - math.pi/angle_div
		elif (y_curr - y_last_temp) >= 0 and (x_curr - x_last_temp) < 0:
			ang_start = math.pi/2 - angle_temp11
			ang_end = 3*math.pi/2 - angle_temp11
			circle_increm = math.pi/angle_div
		else:
			ang_start = math.pi/2 + angle_temp11
			ang_end = 3*math.pi/2 + angle_temp11
			circle_increm = math.pi/angle_div

		if temp_flag1 == 1:
			ang_start = ang_start - math.pi/2
			ang_end = ang_end - math.pi/2

		x_init_temp = circle_start_pt_temp[0] + self.circle_search_r0*math.cos(ang_start)
		y_init_temp = circle_start_pt_temp[1] + self.circle_search_r0*math.sin(ang_start)

		pt_list_temp = [(x_init_temp, y_init_temp)]

		angle_index = ang_start
		for index in range(1,(angle_div+1)):
			angle_index = angle_index + circle_increm
			x_init_temp = circle_start_pt_temp[0] + self.circle_search_r0*math.cos(angle_index)
			y_init_temp = circle_start_pt_temp[1] + self.circle_search_r0*math.sin(angle_index)
			pt_temp = (x_init_temp, y_init_temp)
			pt_list_temp.append(pt_temp)

		x_init_temp = circle_start_pt_temp[0]
		y_init_temp = circle_start_pt_temp[1]
		pt_temp = (x_init_temp, y_init_temp)
		pt_list_temp.append(pt_temp)

		circle_search_pt_list_temp = pt_list_temp

		#print pt_list_temp

		return circle_search_pt_list_temp


	##### Global Search Function: #####
	# This function moves the multicopter towards the next point in the global search list
	# Once it reaches the goal point, it will update the next global search point
	def global_search(self, x_curr, y_curr):
		x_dist = math.sqrt((x_curr - self.global_search_pt_list[0][0])*(x_curr - self.global_search_pt_list[0][0])+(y_curr - self.global_search_pt_list[0][1])*(y_curr - self.global_search_pt_list[0][1]))

		if x_dist <= 0.75:
			self.global_search_pt_list = self.global_search_pt_list[1:]

		new_x_c_1, new_y_c_1 = self.global_search_pt_list[0][:]

		#print "Global Search"
		return new_x_c_1, new_y_c_1


	##### Circle Search Function: #####
	# This function moves the multicopter towards the next point in the circle search list
	# Once it reaches the goal point, it will update the next circle search point
	def circle_search(self, x_curr, y_curr):
		x_dist = math.sqrt((x_curr - self.circle_search_pt_list[0][0])*(x_curr - self.circle_search_pt_list[0][0])+(y_curr - self.circle_search_pt_list[0][1])*(y_curr - self.circle_search_pt_list[0][1]))

		if x_dist <= 0.75 and len(self.circle_search_pt_list) != 1:
			self.circle_search_pt_list = self.circle_search_pt_list[1:]

		if len(self.circle_search_pt_list) == 1:
			self.circle_flag = 0

		new_x_c_2, new_y_c_2 = self.circle_search_pt_list[0][:]

		#print "Circle Search"
		return new_x_c_2, new_y_c_2


	##### Move to Max Function: #####
	# This function moves the multicopter towards the next point in the mtm search list.
	# Once it reaches the goal point, it will update the next mtm search point.
	def move_to_max_signal(self, x_last_temp, y_last_temp, x_curr, y_curr):

		if (x_curr - x_last_temp) == 0:
			if y_curr >= y_last_temp:
				new_y_c_3 = y_curr + 5
			else:
				new_y_c_3 = y_curr - 5

			new_x_c_3 = x_curr
		
		else:
			if x_curr >= x_last_temp:
				new_x_c_3 = x_curr + 5
			else:
				new_x_c_3 = x_curr - 5

			slope_temp = (y_curr - y_last_temp)/(x_curr - x_last_temp)
			b_temp = y_curr - slope_temp*x_curr

			new_y_c_3 = slope_temp*new_x_c_3 + b_temp

		#print "Move to Max Signal"
		return new_x_c_3, new_y_c_3

	##### MAIN FUNCTION BEING CALLED FOR THE CONTROL STRATEGY: #####
	# This function takes the current input values and will return (x_control',y_control') - the new points to move towards...
	# The control logic is based on a strategy that uses a triangle search when no signal is present, does a local circle search...
	# once a signal is found to find the best local signal, and moves in the direction of the best known signal.

	def decide(self, x_curr, y_curr, s_curr, t_curr):
		if self.best_known_signal < s_curr:
			self.best_known_signal = s_curr
			self.best_known_loc = (x_curr, y_curr)

		if self.curr_strat == 3 and s_curr < self.s_last:
			self.circle_flag == 0

		if self.best_known_signal < self.max_signal:
			if self.curr_strat == 1: # If I'm previously doing a global search, continue with global
				if s_curr > self.min_signal:
					self.curr_strat = 2
					# If I'm switching strategies, I will generate a new set of circle pts
					self.circle_flag = 1
					self.local_best_signal = s_curr
					self.local_best_loc = (self.circle_start_pt[0], self.circle_start_pt[1])
					self.circle_start_pt = (x_curr, y_curr)
					self.circle_search_pt_list = self.define_circle_pts(self.circle_start_pt, self.x_last, self.y_last, x_curr, y_curr, 0)
					x_curr_new, y_curr_new = self.circle_search(x_curr, y_curr)
				else:
					self.curr_strat = 1
					x_curr_new, y_curr_new = self.global_search(x_curr, y_curr)

			elif self.curr_strat == 2 or self.curr_strat == 3:
				
				if self.circle_flag == 1:
					if self.curr_strat == 2:
						self.curr_strat = 2
						x_curr_new, y_curr_new = self.circle_search(x_curr, y_curr)
						
						if self.local_best_signal < s_curr:
							self.local_best_signal = s_curr
							self.local_best_loc = (x_curr, y_curr)
						
					else:
						self.curr_strat = 3
						x_curr_new, y_curr_new = self.move_to_max_signal(self.x_last, self.y_last, x_curr, y_curr)
				else:
					if self.curr_strat == 2:
						if self.min_signal < self.local_best_signal:
							self.curr_strat = 3
							# If I'm switching strategies, I use the initial pt of circle and my last position to calculate direction:
							x_curr_new, y_curr_new = self.move_to_max_signal(self.circle_start_pt[0], self.circle_start_pt[1], self.local_best_loc[0], self.local_best_loc[1])
						else:
							self.curr_strat = 1
							x_curr_new, y_curr_new = self.global_search(x_curr, y_curr)
					else:
						self.curr_strat = 2
						# If I'm switching strategies, I will generate a new set of circle pts:
						self.circle_flag = 1
						self.local_best_signal = s_curr
						self.local_best_loc = (self.circle_start_pt[0], self.circle_start_pt[1])
						self.circle_start_pt = (x_curr, y_curr)
						self.circle_search_pt_list = self.define_circle_pts(self.circle_start_pt, self.x_last, self.y_last, x_curr, y_curr, 1)
						x_curr_new, y_curr_new = self.circle_search(x_curr, y_curr)
			else:
				# We don't have an else right now!
				# Possibly a strategy once we find the "local" max?
				x_curr_new, y_curr_new = (0,0)
		else:
			x_curr_new, y_curr_new = self.best_known_loc
			

		if x_curr_new < 0:
			x_curr_new = 0
		elif x_curr_new > self.area_w:
			x_curr_new = self.area_w

		if y_curr_new < 0:
			y_curr_new = 0
		elif y_curr_new > self.area_h:
			y_curr_new = self.area_h

		print "Curr Loc = ", x_curr, y_curr
		print "Last Signal = ", self.s_last
		print "Curr Signal = ", s_curr
		print "Curr Strat = ", self.curr_strat
		print "New Loc = ", x_curr_new, y_curr_new
		print " "

		# Here, we resign our function values for the next time step:
		self.s_last = s_curr
		self.x_last = x_curr
		self.y_last = y_curr

		return x_curr_new, y_curr_new, 1



