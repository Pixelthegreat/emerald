#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: October 15th, 2025
# Purpose: Test classes in Emerald
#
include 'em/string.em'

class Animal then
	func _initialize(this, name, speech) then

		let this.name = name
		let this.speech = speech
	end
	func speak(this) then

		puts string.format('The {} says \'{}\'', this.name, this.speech)
	end
	func _toString(this) then

		return string.format('<Animal \'{}\'>', this.name)
	end
	func _call(this, param) then

		return param
	end
end

class Cow of Animal then
	func _initialize(this) then

		Animal._initialize(this, 'Cow', 'Moo')
	end
	func speak(this) then

		puts '(debug message)'
		return Animal.speak(this)
	end
end

let animal = Cow()
animal.speak()

puts animal, animal('A')
