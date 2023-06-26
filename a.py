import json

value = json.loads(open("a.json").read())
# value = json.loads("""[  "\uD834\uDD1E𝄞" ]""")

print(value)

