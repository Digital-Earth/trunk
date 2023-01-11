require 'dump'
require 'strict'
function iff(cond, a, b) if cond then return a else return b end end

function checkKey(pKey)
    local prefix, digit,  rest  = string.match(pKey, "^([A-T%d]+)-([1-9]?)([0-9]*)$")
    assert(prefix, pKey)
    local n = tonumber(prefix)
    assert(not n or n> 0 and n<=12, pKey)
    local len = 0
    for w in string.gmatch(rest, "0+[1-9]?") do 
        len = len + #w
    end
    assert(len == #rest, pKey)
end

local h = io.open('/log.log','r')
assert(h)




local t = {}
local k = 1
for line in h:lines() do 
    checkKey(line)
    table.insert(t, line)
    if #t == 38  then
        local c=''
        for i,v in ipairs(t)  do
            if #c==0 then
                c=v
            else
                if string.find(line, c,1,true) then c= line end
            end
            print(v, c)
        end 
        t = {}
        k = k + 1
        print(line)
        if k>10 then break end
    end    
    
end

