extends Node
var b2_1 = 0
var b2_2 = 0
var b2_3 = 0
var b2_4 = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass


func _on_button_1_pressed():
	if b2_1 == 1:
		b2_1=0
	else:
		b2_1=1
	pass # Replace with function body.


func _on_button_2_pressed():
	if b2_2 == 1:
		b2_2=0
	else:
		b2_2=1
	pass # Replace with function body.


func _on_button_3_pressed():
	if b2_3 == 1:
		b2_3=0
	else:
		b2_3=1
	pass # Replace with function body.


func _on_button_4_pressed():
	if b2_4 == 1:
		b2_4=0
	else:
		b2_4=1
	pass # Replace with function body.
