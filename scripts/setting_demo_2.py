"""
Set all actions, goals and current world states here!!
this .py file is the only place you will need to change
=======================================================
"""
from action_demo_2 import *

# name, preconditions, effects, position, cost, degree, speed, mode, speed_mode_radius, number, iscup, iswait
def setting(mode):
	if mode is 1:
		a1 = Action('cup_close_1', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a2 = Action('cup_close_2', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a3 = Action('cup_close_3', [], [3], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a4 = Action('cup_close_4', [], [4], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a5 = Action('cup_close_5', [], [1, 3], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a6 = Action('cup_close_6', [], [2, 4], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a7 = Action('cup_close_7', [], [1, 4], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a29 = Action('cup_close_7', [], [2, 3], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a8 = Action('cup_close_all', [], [1, 2, 3, 4], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a9 = Action('cup_open_1', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 2, 0, True)
		a10 = Action('cup_open_2', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 2, 0, True)
		a11 = Action('cup_open_3', [], [3], [], (0, 0), 0, 0, 0, 0, 0, 2, 0, True)
		a12 = Action('cup_open_4', [], [4], [], (0, 0), 0, 0, 0, 0, 0, 2, 0, True)
		a13 = Action('cup_open_5', [], [1, 3], [], (0, 0), 0, 0, 0, 0, 2, 0, 0, True)
		a14 = Action('cup_open_6', [], [2, 4], [], (0, 0), 0, 0, 0, 0, 0, 2, 0, True)
		a15 = Action('cup_open_all', [], [1, 2, 3, 4], [], (0, 0), 0, 0, 0, 0, 0, 2, 0, True)
		a16 = Action('windsock_hand_up', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 5, 0, True)
		a17 = Action('windsock_hand_down', [], [0], [], (0, 0), 0, 0, 0, 0, 0, 5, 0, True)
		a18 = Action('flag', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 6, 0, True)
		a19 = Action('goto', [], [], [], (0, 0), 0, 0, 0, 0, 0, 7, 0, True)
		a20 = Action('lift_left', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 8, 0, True)
		a21 = Action('lift_right', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 9, 0, True)
		a22 = Action('lower_left', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 10, 0, True)
		a23 = Action('lower_right', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 11, 0, True)
		a24 = Action('lower_both', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 12, 0, True)
		a25 = Action('auto_grab_1', [], [1], [], (0, 0), 0, 0, 0, 0, 0, 13, 0, True)
		a26 = Action('auto_grab_2', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 13, 0, True)
		a27 = Action('auto_grab_3', [], [3], [], (0, 0), 0, 0, 0, 0, 0, 13, 0, True)
		a28 = Action('auto_grab_4', [], [4], [], (0, 0), 0, 0, 0, 0, 0, 13, 0, True)
		a30 = Action('open_all', [], [], [], (0, 0), 0, 0, 0, 0, 0, 14, 0, True)
		
		#become modify
		a31 = Action('cup_close_"2"', [], [2], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		a32 = Action('cup_close_"3"', [], [3], [], (0, 0), 0, 0, 0, 0, 0, 1, 0, True)
		
		

		child_list = [a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,
					  a22, a23, a24, a25, a26, a27, a28]

		#   pos mode grab
		c1 = Action('prepare_1', [], [], [a19], (917, 245), 40, 300, 0, 0, 0, 0, 0, True)
		c2 = Action('cup_pos_mode_1', [], [], [a6], (1127, 423), 40, 300, 0, 1, 15, 2, 1, True)
		c3 = Action('prepare_2', [], [], [a19], (685, 248), 143, 300, 0, 0, 0, 0, 0, True)
		c4 = Action('cup_pos_mode_2', [], [], [a5], (470, 415), 143, 300, 0, 1, 15, 2, 1, True)

		#   lift left
		c5 = Action('lift_left_layer', [], [], [a20], (251, 410), 90, 300, 3, 1, 15, 1, 4, False)

		#   pos mode grab
		c6 = Action('cup_pos_mode_3', [], [], [a1], (248, 672), 90, 300, 0, 1, 15, 2, 1, True)

		#   lift right
		c7 = Action('lift_right_layer', [], [], [a21], (248, 672), 90, 300, 3, 1, 15, 1, 4, False)

		#   pos mode grab
		c8 = Action('cup_pos_mode_3', [], [], [a4], (245, 983), 90, 300, 0, 1, 15, 2, 1, True)



		c9 = Action('blocked', [], [], [a19], (350, 1615), 80, 300, 0, 0, 0, 0, 0, True)
		c10 = Action('cup_close_"2"', [], [], [a19], (537, 885), 31, 300, 0, 0, 0, 0, 0, True)
		c11 = Action('cup_prepare_5_2', [], [], [a31], (935, 1123), 31, 300, 0, 0, 0, 0, 0, True)
		c12 = Action('cup_close_"3"', [], [], [a32], (1215,1266), 31, 300, 0, 1, 15, 2, 1, True)
		c13 = Action('lift_both_layer', [], [], [a20, a21], (1350, 1800), 0, 300, 0, 0, 0, 0, 0, True)
		c14 = Action('put_1', [], [], [a15], (1834, 1800), 0, 300, 0, 0, 0, 0, 0, True)
		c15 = Action('put_2', [], [], [a24, a15], (1700, 1800), 0, 300, 0, 0, 0, 0, 0, True)
		c16 = Action('put_3', [], [], [a24, a15], (1550, 1800), 0, 300, 0, 0, 0, 0, 0, True)

		#   push windsock
		c17 = Action('hand_close_all', [], [], [a30], (1300, 1800), 90, 300, 0, 0, 0, 0, 0, True)
		c18 = Action('hand_goto', [], [], [a19], (1300, 700), 90, 300, 0, 0, 0, 0, 0, True)
		c19 = Action('hand_goto2', [], [], [a19], (1808, 300), 270, 300, 0, 0, 0, 0, 0, True)
		c20 = Action('hand_prepare', [], [], [a16], (1808, 155), 270, 300, 3, 1, 15, 4, 4, True)
		c21 = Action('hand_push', [], [], [a17], (1807, 781), 270, 300, 3, 1, 15, 4, 4, True)

		#   pos mode grab
		c22= Action('go_default_1', [], [], [a19], (1618, 486), 220, 300, 3, 1, 15, 4, 4, True)	
		c23 = Action('go_default_2', [], [], [a19], (854, 988), 270, 300, 3, 1, 15, 4, 4, True)

		#   go home north
		c24 = Action('go_home_setup_north', [], ['at_home'], [a19], (800, 800), 90, 300, 3, 1, 15, 4, 4, True)
		c25 = Action('go_home_north', [], ['at_home'], [a19], (300, 800), 270, 300, 3, 1, 15, 4, 4, True)
		c26 = Action('raise_flag', ['at_home'], ['flag_raised', 'go_home'], [a18], (300, 300), 270, 300, 3, 1, 15, 4, 4,True)
		
		#   go home south
		c27 = Action('go_home_setup_south', [], ['at_home'], [a19], (700, 300), 90, 300, 3, 1, 15, 4, 4, True)
		c28 = Action('go_home_south', [], ['at_home'], [a19], (700, 300), 90, 300, 3, 1, 15, 4, 4, True)
		c29 = Action('raise_flag', ['at_home'], ['flag_raised', 'go_home'], [a18], (700, 300), 90, 300, 3, 1, 15, 4, 4,True)

		demo_path = [c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17,c18, c19, c20, c21 ,c22 ,c23]
		go_home_path_north = [c24, c25, c26]
		go_home_path_south = [c27, c28, c29]

	return demo_path, go_home_path_north, go_home_path_south


