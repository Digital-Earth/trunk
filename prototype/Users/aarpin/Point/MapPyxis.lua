require 'dump'
require 'strict'

local MAX_LEVEL = 38
local min = 1
local max = -1

local h = io.open('log.log','r')
assert(h)

function readOneKey(t)
    local t = {}
    for j = 1, MAX_LEVEL do
        table.insert(t, h:read())
    end
    return t
end

local indexes = {}

local data
if max < min then data = h:read('*a') end

if data then
    local t = {}
    for line in string.gmatch(data..'\n', "(.-)\r?\n") do
        table.insert(t, line)
        if #t==MAX_LEVEL then table.insert(indexes,t) t ={} end
    end
else
    for i= 1, max do
        if i == min then indexes = {} end
        table.insert(indexes, readOneKey(indexes))
    end
end

function encodeIndexn(ix)
    assert(ix)
    local l , r = string.match(ix,'(.-)-(.*)')
    local n = tonumber(l)
    if n and n >= 10 then
        l = ({'!','@','#'}) [n-9] -- remap left side
    end
    return l .. r
end

function nextPointerFun()
    local k=0
    return function (t, index)
        if not t then return k end
        if not t[index] then 
            t[index] =  {} 
            k=k+1
        end
        return t[index]
    end
end

function MapKeys(level, limit, numberOfChar)
    local numberOfChar = numberOfChar or 10
    local nextPointer = nextPointerFun()
    local store = {}    
    limit = limit or math.huge
    collectgarbage("collect")
    for i,v in ipairs(indexes)  do
        local ei = encodeIndexn(v[level])
        local left,ei = string.match(ei, '(.)(.*)')
        
        local t = nextPointer(store, left)
        repeat
            left, ei = string.match(ei, '('..string.rep('.',math.min(#ei, numberOfChar))..')(.*)')
            t = nextPointer(t, left)
        until #ei == 0
        if i >= limit then break end       
    end
    return nextPointer()
end


local limit = 1
local result = {}
local limits = {}
local minLevel, maxLevel = 2, MAX_LEVEL
repeat
    limit = limit * 10
    result[limit]={}
    table.insert(limits, limit)
    for level= minLevel, maxLevel  do
        result[limit][level] = {MapKeys(level, limit, 9), MapKeys(level, limit, 10)}
    end 
until limit > #indexes

local h1 = string.rep(' ',10)..','
local h2 = h1
for _,v in ipairs(limits)  do   
   h1 = h1..string.format('%10d,%10s,', v, ' ')
   h2 = h2..string.format('%10s,%10s,', "Pack 9", "Pack 10")
end 

print(h1)
print(h2)

for level= minLevel, maxLevel  do
    local line = string.format('Level:%02d,', level)
    for _, limit in ipairs(limits)  do
        line = line..string.format('%10d,%10d,', unpack(result[limit][level]))
    end
    print(line)
end
