
--[========[Unit Test Helpers]========]

if rawget(_G, 'unit_test') ~= nil then
	print("Loading unit_test module...")

	function unit_test.finish(result)
		wesnoth.wml_actions.endlevel{
			test_result = result,
			linger_mode = true
		}
	end

	function unit_test.fail()
		unit_test.finish('fail')
	end

	function unit_test.pass()
		unit_test.finish('pass')
	end

	function unit_test.tostring(val)
		-- This exists so custom behaviour can be added for specific types if required.
		if type(val) == 'string' then
			-- Strings are output quoted but without escaped_split
			-- Pick a quote style that the string doesn't contain
			if val:find("'") == nil then
				return "'" .. val .. "'"
			elseif val:find('"') == nil then
				return '"' .. val .. '"'
			else
				local n = 0;
				-- This loop is guaranteed to end eventually, although it could run a very large number of times on particularly pathological input.
				while true do
					local left = '[' .. string.rep('=', n) .. '['
					local right = ']' .. string.rep('=', n) .. ']'
					if val:find(left) == nil and val:find(right) == nil then
						return left .. val .. right
					end
					n = n + 1
				end
			end
		end
		return tostring(val)
	end

	function unit_test.log(prefix, message)
		std_print(prefix .. ': ' .. message)
	end

	function unit_test.fail_if(condition, message)
		if condition then
			std_print('Failed because fail_if condition was true:', message)
			unit_test.fail()
		else
			unit_test.pass()
		end
	end

	function unit_test.pass_if(condition, message)
		if condition then
			unit_test.pass()
		else
			std_print('Failed because pass_if condition was false:', message)
			unit_test.fail()
		end
	end

	function unit_test.assert(condition, message)
		if not condition then
			std_print('Assertion failed: ' .. message)
			unit_test.fail()
		end
	end

	function unit_test.assert_not(condition, message)
		if condition then
			std_print('Negative assertion failed:', message)
			unit_test.fail()
		end
	end
	
	function unit_test.assert_throws(a, b, c)
		local fcn, expect_err, message
		if type(a) == 'function' then
			fcn, message = a, b
		else
			fcn, expect_err, message = a, b, c
		end
		local result, err = pcall(fcn)
		if result ~= false then
			if expect_err ~= nil then
				if err ~= expect_err then
					std_print('Assertion failed (should be an error: ' .. unit_test.tostring(expect_err) .. '):', message)
					unit_test.fail()
				end
			else
				std_print('Assertion failed (should be an error):', message)
				unit_test.fail()
			end
		end
	end
	
	function unit_test.assert_nothrow(fcn, message)
		local result, err = pcall(fcn)
		if result ~= true then
			std_print('Assertion failed (should not be an error):', message)
			std_print('  The following error was raised:', err)
			unit_test.fail()
		end
	end

	function unit_test.assert_equal(a, b, message)
		if a ~= b then
			local expr = ('expected %s == %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end
	
	function unit_test.assert_not_equal(a, b, message)
		if a == b then
			local expr = ('expected %s ~= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end

	function unit_test.assert_greater(a, b, message)
		if a <= b then
			local expr = ('expected %s > %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end

	function unit_test.assert_less(a, b, message)
		if a >= b then
			local expr = ('expected %s < %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end

	function unit_test.assert_greater_equal(a, b, message)
		if a < b then
			local expr = ('expected %s >= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end

	function unit_test.assert_less_equal(a, b, message)
		if a > b then
			local expr = ('expected %s <= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end

	function unit_test.assert_in_range(val, min, max, message)
		if val < min or val > max then
			local expr = ('expected %s <= %s <= %s'):format(unit_test.tostring(min), unit_test.tostring(val), unit_test.tostring(max))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end

	function unit_test.assert_approx_equal(a, b, tolerance, message)
		if math.abs(a - b) > tolerance then
			local expr = ('expected %s == %s (within %s)'):format(unit_test.tostring(a), unit_test.tostring(b), unit_test.tostring(tolerance))
			std_print('Assertion failed (' .. expr .. '):', message)
			unit_test.fail()
		end
	end
	
	function unit_test.assert_contains(source, fragment, message)
		if not string.find(source, fragment, 1, true) then
			std_print('Assertion failed (' .. source .. ' contains ' .. fragment .. '):', message)
			unit_test.fail()
		end
	end
end