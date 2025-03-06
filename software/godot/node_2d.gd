extends Node2D
signal catch
signal b1
signal b2
signal b3
signal b4
signal b5
signal b6
signal b7
signal b8
signal b9
signal b10
signal b11
signal b12
signal b13
signal b14
signal b15
signal b16
# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	#catch.emit()
	pass


func _on_node_2d_data_received(data):
	var inp = JSON.parse_string(data)
	
	#
	#if inp["button"][15] != 0:
	print(inp["button"])
	pass # Replace with function body.
