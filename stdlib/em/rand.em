#
# Copyright 2025-2026, Elliot Kohlmyer
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Random number generation
#
include 'em/array.em'

let rand = {}

# Xorshift-based random number generator #
class rand_Xorshift32RNG then

	func _initialize(this) then

		let this.x = array.Array(1, array.unsignedInt)
		this.seed(1)
	end

	func seed(this, value) then

		let this.x[0] = value
	end

	func next(this) then

		let this.x[0] = this.x[0] ^ (this.x[0] << 13)
		let this.x[0] = this.x[0] ^ (this.x[0] >> 17)
		let this.x[0] = this.x[0] ^ (this.x[0] << 5)
		return this.x[0]
	end

	func nextInteger(this, s, e) then

		return this.next() % (e - s) + s
	end

	func nextFloat(this, s, e) then

		return this.next() / 4294967295.0 * (e - s) + s
	end
end
let rand.Xorshift32RNG = rand_Xorshift32RNG

# Create default rng #
let rand.createRNG = func() then

	return rand.Xorshift32RNG()
end
