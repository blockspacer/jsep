local Promise = dofile('promise.lua')
assert(Promise)
local result
local test1 = {
    function()
        local r={}
        Promise.resolve()
        :next(function() table.insert(r, 1) end) -- called
        :next(function() table.insert(r, 3) end)
        :catch(function()table.insert(r, 4) end)
        table.insert(r, 2)
        r = table.concat(r)
        assert(r == '12', r)
    end,
    function()
        local r={}
        Promise.reject()
        :next(function() table.insert(r, 1) end)
        :next(function() table.insert(r, 3) end)
        :catch(function()table.insert(r, 4) end) -- called
        table.insert(r, 2)
        r = table.concat(r)
        assert(r == '42', r)
    end,
    function()
        local r={}
        Promise.new(function(resolve, reject)
            table.insert(r, 1)
            resolve()
            table.insert(r, 2)
        end)
        :catch(function()table.insert(r, 5) end) --skip
        :next(function() table.insert(r, 3); return 4 end)
        :next(function(v) table.insert(r, v) end)
        table.insert(r, 5)
        r = table.concat(r)
        assert(r == '12345', r)
    end,
    function()
        local r={}
        Promise.resolve(1)
        :next(function() error(v, 0) end)
        :next(function() table.insert(r, 2) end) --skip
        :catch(function()table.insert(r, 5) end)
        table.insert(r, 3)
        r = table.concat(r)
        assert(r == '53', r)
    end,
    function(i)
        local r={}
        local p1= Promise.new('A'..i)
        Promise.race{p1,Promise.new('N')}
        :next(function() assert(false) end, 'B')
        :catch(function() return 'o' end, 'C') -- return to continue
        :next(function(v) error(v..'k', 0) end, 'D')
        :catch(function(v) print(v) end, 'E')
        p1:reject()
    end
}

function all()
    local promise1, promise2, promise3 = Promise.new(), Promise.new(), Promise.new()
    Promise.all(promise1, promise2, promise3)
    :next(function(values)
        print(values[1],values[2],values[3])
        print('All promises have resolved')
    end)
    :catch(function(values)
        print(values[1],values[2],values[3])
        print('At least one promise was rejected')
    end)
    promise1:reject()
    promise2:resolve(2)
    promise3:resolve(3)
end
function chain()
    local promise1 = Promise.new()
    promise1.id = 0
    promise1
    :next(function(v, self)
        --return 'hello'
        self.id=1
        self:resolve('hello')
    end)
    :next(function(v, self)
        self.id=2
        self:resolve(v .. ' world')
    end)
    :catch(function(v, self)
        self.id=4
        print('catch', v)
    end)
    :next(function(v, self)
        self.id=3
        print('final', v)
    end)
    promise1:resolve('')
end
function const()
    Promise.reject('fail'):next(
    function() print('not called') end,
    function(reason) print(reason)end
    )
end
for i, t in ipairs(test1) do t(i) end
for i=1,10 do 
    print(string.rep('-', 60))
    Promise.update() 
end


