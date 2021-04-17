
--[========[Unit Test Helpers]========]

if rawget(_G, 'unit_test') ~= nil then
	print("Loading unit_test module...")
	
	--! Causes the test to complete with a specific result.
	--! This is mostly an internal detail but might very occasionally be useful to produce a result other than pass or fail
	function unit_test.finish(result)
		wesnoth.wml_actions.endlevel{
			test_result = result,
			linger_mode = true
		}
	end

	--! Fail the test unconditionally
	function unit_test.fail()
		unit_test.finish('fail')
	end

	--! Pass the test unconditionally
	function unit_test.pass()
		unit_test.finish('pass')
	end

	--! Convert a value to a string for output
	--! Strings are output quoted, all other types just use tostring
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

	--! Output a log message to the test output
	function unit_test.log(prefix, message)
		std_print(prefix .. ': ' .. message)
	end

	--! If condition is true, fail the test with message,otherwise pass the test.
	--! Can be thought of as 'return not condition'
	function unit_test.fail_if(condition, message)
		if condition then
			unit_test.log('Failed because fail_if condition was true', message)
			unit_test.fail()
		else
			unit_test.pass()
		end
	end

	--! If condition is true, pass the test with message, otherwise fail the test.
	--! Can be thought of as 'return condition'
	function unit_test.pass_if(condition, message)
		if condition then
			unit_test.pass()
		else
			unit_test.log('Failed because pass_if condition was false', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless the condition is true
	function unit_test.assert(condition, message)
		if not condition then
			unit_test.log('Assertion failed', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless the condition is false
	function unit_test.assert_not(condition, message)
		if condition then
			unit_test.log('Negative assertion failed', message)
			unit_test.fail()
		end
	end
	
	--! Fail the test with a message unless the function exits with any error
	function unit_test.assert_throws(fcn, message)
		local result = pcall(fcn)
		if result ~= false then
			unit_test.log('Assertion failed (should be an error)', message)
			unit_test.fail()
		end
	end
	
	--! Fail the test with a message unless the function exits with a specific error
	function unit_test.assert_throws_with(expect_err, fcn, message)
		local result, err = pcall(fcn)
		if result ~= false and err ~= expect_err then
			unit_test.log('Assertion failed (should be an error: ' .. unit_test.tostring(expect_err) .. ')', message)
			unit_test.fail()
		end
	end
	
	--! Fail the test with a message unless the function exits with no errors
	function unit_test.assert_nothrow(fcn, message)
		local result, err = pcall(fcn)
		if result ~= true then
			unit_test.log('Assertion failed (should not be an error)', message)
			unit_test.log('  The following error was raised', unit_test.tostring(err))
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless a == b
	function unit_test.assert_equal(a, b, message)
		if a ~= b then
			local expr = ('expected %s == %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end
	
	--! Fail the test with a message unless a ~= b
	function unit_test.assert_not_equal(a, b, message)
		if a == b then
			local expr = ('expected %s ~= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless a > b
	function unit_test.assert_greater(a, b, message)
		if a <= b then
			local expr = ('expected %s > %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unti_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless a < b
	function unit_test.assert_less(a, b, message)
		if a >= b then
			local expr = ('expected %s < %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless a >= b
	function unit_test.assert_greater_equal(a, b, message)
		if a < b then
			local expr = ('expected %s >= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless a <= b
	function unit_test.assert_less_equal(a, b, message)
		if a > b then
			local expr = ('expected %s <= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless val is in the range [min, max]
	function unit_test.assert_in_range(val, min, max, message)
		if val < min or val > max then
			local expr = ('expected %s <= %s <= %s'):format(unit_test.tostring(min), unit_test.tostring(val), unit_test.tostring(max))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	--! Fail the test with a message unless a == b within the tolerance
	--! Intended for comparing real numbers
	function unit_test.assert_approx_equal(a, b, tolerance, message)
		if math.abs(a - b) > tolerance then
			local expr = ('expected %s == %s (within %s)'):format(unit_test.tostring(a), unit_test.tostring(b), unit_test.tostring(tolerance))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end
	
	--! Fail the test with a message unless the source string contains the fragment as a substring
	function unit_test.assert_contains(source, fragment, message)
		if not string.find(source, fragment, 1, true) then
			unit_test.log('Assertion failed (' .. source .. ' contains ' .. fragment .. ')', message)
			unit_test.fail()
		end
	end
end