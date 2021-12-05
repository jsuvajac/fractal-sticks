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

def draw_fractal(instructions: str):
	''' draws instructions a variable at a time using turtle '''

	FORWARD_DISTANCE = 25
	color('green')
	# face the turtle due north
	setheading(90)
	# move down from center
	speed(0)
	up()
	setpos((0, -500))
	down()

	stack = []
	for x in list(instructions):
		if x == "X":
			if len(stack) > 0:
				right(5*len(stack))
			# continue
		elif x == "F":
			forward(FORWARD_DISTANCE)
		elif x == "-":
			right(25)
		elif x == "+":
			left(25)
		elif x == "[":
			stack.append((pos(), heading()))
		elif x == "]":
			(position, angle) = stack.pop()
			up()
			setpos(position)
			down()
			setheading(angle)

	done()


variables = ["X", "F"]
constants = ["+", "-", "[", "]"]
axiom = "X"
rules = {
	"X": "F+[[X]-X]-F[-FX]+X",
	"F": "FF"
}

# run iterations
next_step = axiom
for i in range(5):
	# print(f"{i}: {next_step}")
	next_step = run_step(rules, next_step, constants)

draw_fractal(next_step)
