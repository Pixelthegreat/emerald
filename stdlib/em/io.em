#
# Copyright 2025-2026, Elliot Kohlmyer
#
# SPDX-License-Identifier: BSD-3-Clause
#
# IO abstraction module
#
include 'em/os.em'

let io = {
	'read': os.read,
	'write': os.write,
	'binary': os.binary,

	'atStart': os.atStart,
	'atCursor': os.atCursor,
	'atEnd': os.atEnd,
}

# Error #
class io_Error of Error then
end
let io.Error = io_Error

# File #
class io_File then

	func _initialize(this, file, mode) then

		let this.file = file
		let this.mode = mode
	end

	func read(this, data) then

		return os.readFile(this.file, data)
	end

	func readString(this, count) then

		let s = ' ' * count
		this.read(s)
		return s
	end

	func write(this, data) then

		return os.writeFile(this.file, data)
	end

	func seek(this, whence, position) then

		return os.seekFile(this.file, whence, position)
	end

	func tell(this) then

		return this.seek(io.atCursor, 0)
	end

	func close(this) then

		return os.closeFile(this.file)
	end
end
let io.File = io_File

# open file #
let io.open = func (path, mode) then

	return io.File(os.openFile(path, mode), mode)
end
