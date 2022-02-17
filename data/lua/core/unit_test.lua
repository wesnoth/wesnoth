
--[========[Unit Test Helpers]========]

if rawget(_G, 'unit_test') ~= nil then
	print("Loading unit_test module...")

	---Causes the test to complete with a specific result.
	---This is mostly an internal detail but might very occasionally be useful to produce a result other than pass or fail
	---@param result string
	function unit_test.finish(result)
		wesnoth.wml_actions.endlevel{
			test_result = result,
			linger_mode = true
		}
	end

	---End the test in unconditional failure
	---This does not immediately terminate the test; any assertions in the current event will still be run.
	---Without calling either succeed() or fail(), the test will never end.
	---This allows a test to run over multiple events and even multiple turns.
	function unit_test.fail()
		unit_test.finish('fail')
	end

	---End the test in success if all assertions passed, otherwise in failure
	---Without calling either succeed() or fail(), the test will never end.
	---This allows a test to run over multiple events and even multiple turns.
	function unit_test.succeed()
		unit_test.finish('pass')
	end

	---Convert a value to a string for output
	---Strings are output quoted, all other types just use tostring
	---This exists to be overridden if a test needs custom output for some type
	---@param val any
	---@return string
	function unit_test.tostring(val)
		-- This exists so custom behaviour can be added for specific types if required.
		if type(val) == 'string' then
			-- Strings are output quoted
			return string.format('%q', val)
		end
		return tostring(val)
	end

	---Output a log message to the test output, in the form "prefix: message"
	---@param prefix string
	---@param message string
	function unit_test.log(prefix, message)
		std_print(prefix .. ': ' .. message)
	end

	-- This is a way to ensure that all assertions contain a descriptive message
	setmetatable(unit_test, {
		__newindex = function(self, key, val)
			if string.sub(key, 1, 6) == 'assert' then
				local info = debug.getinfo(val, 'u')
				local recursion_guard = false
				local underlying_fcn = val
				local fcn = function(...)
					if not recursion_guard then
						-- Last argument is the message
						local message = select(info.nparams, ...)
						recursion_guard = true
						unit_test.assert_not_equal(message, nil, string.format('unit_test.%s missing a message', key))
						recursion_guard = false
					end
					underlying_fcn(...)
				end
				val = fcn
			end
			rawset(self, key, val)
		end
	})

	---Fail the test with a message unless the condition is true
	---@param condition boolean
	---@param message string
	function unit_test.assert(condition, message)
		if not condition then
			unit_test.log('Assertion failed', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless the function exits with any error
	---@param fcn function
	---@param message string
	function unit_test.assert_throws(fcn, message)
		local result = pcall(fcn)
		if result ~= false then
			unit_test.log('Assertion failed (should be an error)', message)
			unit_test.fail()
		end
	end

	local function match_error(expect, have)
		if type(expect) == 'string' then
			local m = string.match(have, '^%[.-%]:%d: (.*)')
			if m then return expect == m end
		end
		return expect == have
	end

	---Fail the test with a message unless the function exits with a specific error
	---@param expect_err any
	---@param fcn function
	---@param message string
	function unit_test.assert_throws_with(expect_err, fcn, message)
		local result, err = pcall(fcn)
		if result ~= false or not match_error(expect_err, err) then
			unit_test.log('Assertion failed (should be an error: ' .. unit_test.tostring(expect_err) .. ' but got ' .. unit_test.tostring(err) .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless the function exits with no errors
	---@param fcn function
	---@param message string
	function unit_test.assert_nothrow(fcn, message)
		local result, err = pcall(fcn)
		if result ~= true then
			unit_test.log('Assertion failed (should not be an error)', message)
			unit_test.log('  The following error was raised', unit_test.tostring(err))
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a == b
	---@param a any
	---@param b any
	---@param message string
	function unit_test.assert_equal(a, b, message)
		if a ~= b then
			local expr = ('expected %s == %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a ~= b
	---@param a any
	---@param b any
	---@param message string
	function unit_test.assert_not_equal(a, b, message)
		if a == b then
			local expr = ('expected %s ~= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a > b
	---@param a any
	---@param b any
	---@param message string
	function unit_test.assert_greater(a, b, message)
		if a <= b then
			local expr = ('expected %s > %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a < b
	---@param a any
	---@param b any
	---@param message string
	function unit_test.assert_less(a, b, message)
		if a >= b then
			local expr = ('expected %s < %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a >= b
	---@param a any
	---@param b any
	---@param message string
	function unit_test.assert_greater_equal(a, b, message)
		if a < b then
			local expr = ('expected %s >= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a <= b
	---@param a any
	---@param b any
	---@param message string
	function unit_test.assert_less_equal(a, b, message)
		if a > b then
			local expr = ('expected %s <= %s'):format(unit_test.tostring(a), unit_test.tostring(b))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless val is in the range [min, max]
	---@param val number
	---@param min number
	---@param max number
	---@param message string
	function unit_test.assert_in_range(val, min, max, message)
		if val < min or val > max then
			local expr = ('expected %s <= %s <= %s'):format(unit_test.tostring(min), unit_test.tostring(val), unit_test.tostring(max))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless a == b within the tolerance
	---Intended for comparing real numbers
	---@param a number
	---@param b number
	---@param tolerance number
	---@param message string
	function unit_test.assert_approx_equal(a, b, tolerance, message)
		if math.abs(a - b) > tolerance then
			local expr = ('expected %s == %s (within %s)'):format(unit_test.tostring(a), unit_test.tostring(b), unit_test.tostring(tolerance))
			unit_test.log('Assertion failed (' .. expr .. ')', message)
			unit_test.fail()
		end
	end

	---Fail the test with a message unless the source string contains the fragment as a substring
	---@param source string
	---@param fragment string
	---@param message string
	function unit_test.assert_contains(source, fragment, message)
		if not string.find(source, fragment, 1, true) then
			unit_test.log('Assertion failed (' .. source .. ' contains ' .. fragment .. ')', message)
			unit_test.fail()
		end
	end
end
