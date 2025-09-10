func fizzbuzz() then
	for i = 1 to 101 then
		let s = ''
		if i % 3 == 0 then let s = s + 'Fizz' end
		if i % 5 == 0 then let s = s + 'Buzz' end
		if not s then let s = i end
		puts s
	end
end
fizzbuzz()
