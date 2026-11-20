#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: November 20th, 2026
# Purpose: Test program execution time
#
include 'em/rand.em'

let rng = rand.createRNG()

let alphabet = 'abcdefghijklmnopqrstuvwxyz'
let map = {}

for i = 0 to 10000 then
	let name = alphabet[rng.nextInteger(0, 26)] + alphabet[rng.nextInteger(0, 26)]
	let map[name] = i
	map[name]
end
