#!/usr/bin/python
#coding=utf-8

"""
将track的..tracklib转化为.trackbin, 二进制文件
"""
"""
In the beginning, Please pardon me for my inadequate English.
 
This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.

This python script is used to convert a tracklib file that includes all the necessary data for fish movements and is generated 
from PathEditor, to a binary file which will be processed and loaded by game engine.

The following Python methods shows how to process certain data structures and files, reading them properly and converting the data
to a compressed data binary stream. 
"""
import os,sys
import struct
import biplist
import config
import util
from  xml.dom import  minidom


s_move_types = {
	"line":0,
	"circle":1,
	"bezier5":2,
	"crspline":3
	}

def convert_vector3d_string(vector3d_like_str, track_bin,isShowMode):
	items = vector3d_like_str.encode("utf-8").strip("()").split(",")
	index = 0
	for item in items:
		index+=1
		if index%3 == 0 and float(item)>=64.0 and isShowMode == "True":
			print("Warning!The depth value is set with a value that is higher than standard height(64.0)!Recommend reset the value!!!")
		track_bin.write(struct.pack("<f", float(item)))

def line(move_element, track_bin,isShowMode):
	time = float(move_element.getAttribute("time"))
	track_bin.write(struct.pack('<f', time))
	
	movetype = move_element.getAttribute("movestyle")
	if movetype != "":
		track_bin.write(struct.pack('<i',int(movetype)))
	else :
		track_bin.write(struct.pack('<i',0))
		
	convert_vector3d_string(move_element.getAttribute("start"), track_bin,isShowMode)
	convert_vector3d_string(move_element.getAttribute("end"), track_bin,isShowMode)

def circle(move_element, track_bin,isShowMode):
	time = float(move_element.getAttribute("time"))
	track_bin.write(struct.pack('<f', time))
	
	movetype = move_element.getAttribute("movestyle")
	if movetype != "":
		track_bin.write(struct.pack('<i',int(movetype)))
	else :
		track_bin.write(struct.pack('<i',0))
	
	total_angle = float(move_element.getAttribute("percentage"))
	track_bin.write(struct.pack('<f', total_angle))
	clock_wise = move_element.getAttribute("clock_wise")
	if clock_wise == "True":
		track_bin.write(struct.pack('<i', 1))
	else :
		track_bin.write(struct.pack('<i', 0))
	convert_vector3d_string(move_element.getAttribute("center_pos"), track_bin,isShowMode)
	
	direction = move_element.getAttribute("nDirection_pos")
	if direction != "":
		convert_vector3d_string(direction, track_bin,isShowMode)
	else:
		convert_vector3d_string("(0.0,1.0,0.0)",track_bin,isShowMode)
	radius = move_element.getAttribute("radius")
	if radius >= 64.0:
		print("Warning!The Radius value over 64.0f")
	if radius != "":
		track_bin.write(struct.pack('<f',float(radius)))
	else:
		track_bin.write(struct.pack('<f',5.0))
		
	beginAngle = move_element.getAttribute("beginAngle")
	if beginAngle != "":
		track_bin.write(struct.pack('<f',float(beginAngle)))
	else:
		track_bin.write(struct.pack('<f',0.0))
		
def bezier5(move_element, track_bin,isShowMode):
	time = float(move_element.getAttribute("time"))
	track_bin.write(struct.pack('<f', time))

	movetype = move_element.getAttribute("movestyle")
	if movetype != "":
		track_bin.write(struct.pack('<i',int(movetype)))
	else :
		track_bin.write(struct.pack('<i',0))
	
	points = move_element.getAttribute("points").split("|")
	for point in points:
		convert_vector3d_string(point, track_bin,isShowMode)

def crspline(move_element, track_bin,isShowMode):
	time = float(move_element.getAttribute("time"))
	track_bin.write(struct.pack('<f', time))
	
	bloop = move_element.getAttribute("loop")
	if	bloop == "True":
		track_bin.write(struct.pack('<i',1))
	else :
		track_bin.write(struct.pack('<i',0))
		
	movetype = move_element.getAttribute("movestyle")
	if movetype != "":
		track_bin.write(struct.pack('<i',int(movetype)))
	else :
		track_bin.write(struct.pack('<i',0))
	
	nodes_count = int(move_element.getAttribute("point_count"))
	track_bin.write(struct.pack('<i',nodes_count))
	
	points = move_element.getAttribute("points").split("|")
	for point in points:
		convert_vector3d_string(point, track_bin,isShowMode)

s_move_fun_map = {
    "line":line,
    "circle":circle,
    "bezier5":bezier5,
    "crspline":crspline
}	

def convert_move(move_element, track_bin,isShowMode):
	move_type = move_element.getAttribute("type")
	track_bin.write(struct.pack("<i",s_move_types[move_type]))
	convert_func = s_move_fun_map[move_type]
	convert_func(move_element,track_bin,isShowMode)
	
def convert_path(path_element, track_bin):
	isShowPath = path_element.getAttribute("show_path")
	if	isShowPath == "True":
		track_bin.write(struct.pack('<i',1))
	else :
		track_bin.write(struct.pack('<i',0))
		
	repeat_times = int(path_element.getAttribute("repeat_times"))
	delay_time = float(path_element.getAttribute("delay_time"))
	track_bin.write(struct.pack('<if', repeat_times,delay_time))
	
	fish_type = int(path_element.getAttribute("fish_type"))
	scale_str = path_element.getAttribute("fish_size")
	if len(scale_str)>0:
		fish_scale = float(scale_str)
	else:
		fish_scale = 1.0
	track_bin.write(struct.pack('<if', fish_type,fish_scale))
			
	event_points = path_element.getAttribute("eventpoints")
	track_bin.write(struct.pack("<i",len(event_points)))
	track_bin.write(event_points)
	
	move_elements = path_element.getElementsByTagName("move")
	track_bin.write(struct.pack("<i", len(move_elements)))
	for move_element in move_elements:
		convert_move(move_element, track_bin,isShowPath)
		
	
def convert_wave(wave_element, track_bin):

	waveDepth = wave_element.getAttribute("depth")
	if waveDepth != "":
		track_bin.write(struct.pack('<f',float(waveDepth)))
	else:
		track_bin.write(struct.pack('<f',-1.0))
	
	path_elements = wave_element.getElementsByTagName("path")
	track_bin.write(struct.pack("<i", len(path_elements)))
	for path_element in path_elements:
		convert_path(path_element, track_bin)

def convert_screen(screen_element, track_bin):
	screenTime = screen_element.getAttribute("customScreenTime")
	if screenTime != "":
		track_bin.write(struct.pack("<f",float(screenTime)))
	else :
		track_bin.write(struct.pack("<f",10.0))
		
	usingCustom = screen_element.getAttribute("usingCustom")
	if usingCustom == "True":
		track_bin.write(struct.pack("<i",1))
	else :
		track_bin.write(struct.pack("<i",0))
		
	wave_elements = screen_element.getElementsByTagName("wave")	
	track_bin.write(struct.pack("<i", len(wave_elements)))
	for wave_element in wave_elements:
		wave_index = int(wave_element.getAttribute("index"))
		wave_delay = float(wave_element.getAttribute("delay"))
		track_bin.write(struct.pack("<if", wave_index, wave_delay))

	
def convert_bin(xml_file, bin_file):
	print "convert:%s"%xml_file
	doc = minidom.parse(xml_file) 
	track_element = doc.documentElement
	
	track_bin = open(bin_file, "wb")
	version = track_element.getAttribute("version")
	track_bin.write(struct.pack("<i",len(version)))
	track_bin.write(version)
	
	screen_time_str = track_element.getAttribute("screen_time")
	screen_time = 1.0
	if screen_time_str != "":
		screen_time = float(screen_time_str)
	
	track_bin.write(struct.pack("<f",screen_time))
	
	screenSelectStrategy = track_element.getAttribute("screen_select_strategy")
	if(screenSelectStrategy):
		track_bin.write(struct.pack("<i",int(screenSelectStrategy)))
	else:
		track_bin.write(struct.pack("<i",int(1)))
	
	#todo
	#track_bin.write(version.	lower())
	#track_bin.write(struct.pack("<f", float(track_element.getAttribute("delay_between_screen"))))
	
	#screens
	screens_element = track_element.getElementsByTagName("screens")[0]
	screen_elements = screens_element.getElementsByTagName("screen")
	track_bin.write(struct.pack("<i", len(screen_elements)))
	for screen_element in screen_elements:
		convert_screen(screen_element, track_bin)
		
	#waves
	waves_element = track_element.getElementsByTagName("waves")[0]
	wave_elements = waves_element.getElementsByTagName("wave")
	track_bin.write(struct.pack("<i", len(wave_elements)))
	for wave_element in wave_elements:
		convert_wave(wave_element, track_bin)
		

def convert(from_dir, to_dir):
	print "convert new track..."
	for item in os.listdir(from_dir):
		if item.lower().endswith(".xml"):
			track_name = item[:-len(".xml")]
			track_file = os.path.join(from_dir, item)
			print "convert track name: %s"%track_name
			convert_bin(track_file, os.path.join(to_dir, "%s.trackbin"%(track_name)))		
		
if __name__ == "__main__":
	if(len(sys.argv) == 3):
		convert(sys.argv[1], sys.argv[2])
	else:
		convert(config.g_track_new_dir, config.g_track_tmp_dir)
		util.copy_dir_to_dir(config.g_track_tmp_dir, config.g_resources_new_track_dir, True)
