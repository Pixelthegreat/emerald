#               #
# String module #
#               #
let string = __module_string

# Left justify #
let string.justifyLeft = func(s, c, w) then

	if lengthOf(s) >= w then
		return s
	end
	return s + c * (w - lengthOf(s))
end

# Right justify #
let string.justifyRight = func(s, c, w) then

	if lengthOf(s) >= w then
		return s
	end
	return c * (w - lengthOf(s)) + s
end

# Center justify  #
let string.justifyCenter = func(s, c, w) then

	if lengthOf(s) >= w then
		return s
	end
	return c * ((w - lengthOf(s)) / 2) + s
end
