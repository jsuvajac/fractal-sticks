from turtle import *

def run_step(rules: dict, step: str, constants: list) -> str:
	''' generates the next step '''
	next_step = []
	for x in list(step):
		if x in constants:
			next_step.append(x)
		else:
			next_step.append(rules[x])
	return "".join(next_step)
	
def draw_fractal(instructions: str, angle: int):
	''' draws instructions a variable at a time using turtle '''
	COLOURS = ["green", "green1", "green2", "green3", "green4", "green4", "green4", "green4",]

	FORWARD_DISTANCE = 10
	# color('green')
	# face the turtle due north
	setheading(90)
	# move down from center
	speed(0)
	up()
	setpos((0, 0))
	down()
	# max_size = 0
	stack = []
	for x in list(instructions):
		# if len(stack) > max_size:
		# 	max_size = len(stack)
		# 	print(max_size)
		# pensize((11 - len(stack))// 2)
		color(COLOURS[len(stack) // 2])
		if x == "X":
			if len(stack) > 0:
				right(5*len(stack))
			# continue
		elif x == "F":
			forward(FORWARD_DISTANCE)
			print(pos(), heading())
		elif x == "-":
			right(angle)
		elif x == "+":
			left(angle)
		elif x == "[":
			stack.append((pos(), heading()))
		elif x == "]":
			(last_position, last_angle) = stack.pop()
			up()
			setpos(last_position)
			down()
			setheading(last_angle)

	done()

constants = ["+", "-", "[", "]"]
axiom = "X"
angle = 25
rules = {
	"X": "F+[[X]-X]-F[-FX]+X",
	# "X": "F+[-FX+F]-[+F-F]+[+F+F]-[-FX-F]",
	# "X": "F+[-X-X-X]-F[-FX]+X",
	"F": "FF"
}

# run iterations
next_step = axiom
for i in range(5):
	# print(f"{i}: {next_step}")
	print(f"{i}: {len(next_step)}")
	next_step = run_step(rules, next_step, constants)

draw_fractal(next_step, angle)
