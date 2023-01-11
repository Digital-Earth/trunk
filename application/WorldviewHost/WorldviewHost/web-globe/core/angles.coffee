degreesToRadiansFactor = Math.PI/180
radiansToDegreesFactor = 180/Math.PI

angles =
	radians: (deg) -> deg * degreesToRadiansFactor
	degrees: (rad) -> rad * radiansToDegreesFactor
	const: {}		

for angle in [30,45,90,135,180,270]
	angles.const["degrees#{angle}InRadians"] = angles.radians(angle)

module.exports = angles