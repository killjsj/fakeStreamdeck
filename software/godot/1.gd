extends Node
signal catch
var b1_1 = 0
var b1_2 = 0
var b1_3 = 0
var b1_4 = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass


func _on_button_1_pressed():
	
	if b1_1 == 1:
		b1_1=0
	else:
		b1_1=1
	pass # Replace with function body.


func _on_button_2_pressed():
	if b1_2 == 1:
		b1_2=0
	else:
		b1_2=1
	pass # Replace with function body.


func _on_button_3_pressed():
	if b1_3 == 1:
		b1_3=0
	else:
		b1_3=1
	pass # Replace with function body.


func _on_button_4_pressed():
	if b1_4 == 1:
		b1_4=0
	else:
		b1_4=1
	pass # Replace with function body.


func _on_button_1_button_down():
	print("a")
	catch.emit()
	pass # Replace with function body.
