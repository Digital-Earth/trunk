require 'dump'
require 'strict'

local min = 1
local max = -1

function iff(cond, a, b) if cond then return a else return b end end

function checkForValidPyxisIndex(pKey)
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

local h = io.open('log.log','r')
assert(h)

function readOneKey(t)
    local t = {}
    for j = 1, 38 do
        table.insert(t, h:read())
    end
    return t
end

indexes = {}

local data
if max < min then data = h:read('*a') end

if data then
    local t = {}
    for line in string.gmatch(data..'\n', "(.-)\r?\n") do 
        table.insert(t, line) 
        if #t==38 then table.insert(indexes,t) t ={} end
    end
else
    for i= 1, max do
        if i == min then indexes = {} end
        table.insert(indexes, readOneKey(indexes))
    end
end

function pDiff(master, line, c)
    local s = ''
    master = master or ''
    line = line or ''
    for i = 1, #line do
        s = s .. iff(string.byte(line,i) == string.byte(master,i), ' ', c)
    end
    return s
end
pl = 0

local dist = {}

function encodeIndex(ix)
    local l , r = string.match(ix,'(.-)-(.*)')
    local n = tonumber(l)
    if n and n >= 10 then
        l = ({'!','@','#'}) [n-9] -- remap left side
    end
    return l .. r
end

function resolvePyxisIndex(level, compressIndex)
    local index, i, len = '', 1, level + 1
    repeat
        assert(i <= #compressIndex)
        local  r, s = unpack(compressIndex[i])
        local l = (math.abs(r) - #s) + 1
        if r< 0 then
            r = - r
            if len <= r then
                s =  compressIndex[i + math.mod(r - len , 2)][2]
            else
                s = compressIndex[i + 1][2]
            end
            i = i + 1
        end
        i = i + 1
        index = string.sub(index, 1, l - 1) .. s
    until #index >= len
    return string.sub(index, 1, len)
end

function compressIndexes(ix)
    
    local xPyxisIndex = {}
    local function print_xPyxisIndex( last )
        for i, v in ipairs(xPyxisIndex)  do
            if not last or i == #xPyxisIndex then  print('xPyxisIndex['..string.format("%2d", i)..']', unpack(v)) end
        end
        print('----------------------------------------------------')
    end
    if ix then
        local prev
        for i, v in ipairs(ix)  do
            local encodedIndex = encodeIndex(v)
            print(encodedIndex, v)
            if i == 1 then
                xPyxisIndex = {{1, #encodedIndex, encodedIndex}}
            elseif prev == string.sub(encodedIndex, 1, #prev) then
                xPyxisIndex[#xPyxisIndex][3] = xPyxisIndex[#xPyxisIndex][3]  .. string.sub(encodedIndex, #prev +1 )
                xPyxisIndex[#xPyxisIndex][2] = xPyxisIndex[#xPyxisIndex][1] + #xPyxisIndex[#xPyxisIndex][3] -1
            else
                xPyxisIndex[#xPyxisIndex + 1]= {0, #encodedIndex, ''}
                for i = 1, #prev do
                    xPyxisIndex[#xPyxisIndex][1] = i
                    if string.byte(prev,i) ~= string.byte(encodedIndex,i) then break end
                end
                xPyxisIndex[#xPyxisIndex][3] = string.sub(encodedIndex, xPyxisIndex[#xPyxisIndex][1],  xPyxisIndex[#xPyxisIndex][2])
            end
            prev = encodedIndex
        end

        print_xPyxisIndex()
        for i= 1, #xPyxisIndex - 2 do
            if xPyxisIndex[i][1] == xPyxisIndex[i+2][1] and xPyxisIndex[i][3] == string.sub(xPyxisIndex[i+2][3], 1, #xPyxisIndex[i][3])  then
                if  i==1 or xPyxisIndex[i-1][3] ~= '=' then
                    if xPyxisIndex[i][1] + 1 == xPyxisIndex[i][2]  then 
                        xPyxisIndex[i][3] = '='
                    end
                elseif xPyxisIndex[i-1][1] == xPyxisIndex[i][1] and xPyxisIndex[i-1][2] + 1 == xPyxisIndex[i][2] then
                    xPyxisIndex[i][3] = '='                    
                end
            end
        end
        
        print_xPyxisIndex()
        local i, sign = 1, false 
        repeat
            if xPyxisIndex[i][3] == '='  then
                table.remove(xPyxisIndex,i)
                sign = true
            else
                table.remove(xPyxisIndex[i],1)
                if sign then 
                    xPyxisIndex[i][1] = -xPyxisIndex[i][1] 
                    sign = false
                end
                i = i + 1
            end
        until i > #xPyxisIndex
        
        print_xPyxisIndex()   
        
        local tl = 0
        for i,v in ipairs(xPyxisIndex)  do
            tl = tl + 1 + #(xPyxisIndex[i][2] or '')
        end
        print('Total length: ', tl)
        dist[tl] = (dist[tl] or 0) + 1
        return xPyxisIndex
    end
end

function verifyKey(pyxisIndexes, compressIndex)
    for i,v in ipairs(pyxisIndexes)  do
        local encodedIndex, ix = encodeIndex(v), resolvePyxisIndex(i , compressIndex)
        assert( encodedIndex == ix, '\n'.."expected:"..encodedIndex .. ' ~= ' .. ix)
    end
end

master,  currentPyxisIndexes ='', nil

function pIndex(ix)
    print(ix,#string.match(ix, '-(.*)')) -- index and level
end
print('---')
for n, v in ipairs(indexes)  do
    print('------------------- new index', n )
    if math.mod(n, 100) == 0 then io.stderr:write(string.format("%d ", n)) end
    
    while #v > 0 do
        line = table.remove(v)
        checkForValidPyxisIndex(line)
        if #v == 37 then -- new index
            master, prevPyxisIndex = line, nil
            pIndex(line)
            currentPyxisIndexes = {line}
        else
            table.insert(currentPyxisIndexes, 1, line)
            pIndex(line)
            if string.sub(master, 1, #line) ~= line then
                local s = pDiff(master, line, '*')
                if prevPyxisIndex then
                    local d = pDiff(prevPyxisIndex, line, '+')
                    if string.find(d, '+') then s = s..'\n'..d end
                end
                prevPyxisIndex = line
            end
        end
        pl = #line
    end
    verifyKey(currentPyxisIndexes, compressIndexes(currentPyxisIndexes))
end


local grandTotal = 0
for i= 1, table.maxn(dist) do
    if dist[i] then grandTotal = grandTotal + i*dist[i] end
    if grandTotal > 0 then print(i,dist[i] or 0) end
end

print('Average length: ', grandTotal / #indexes,'for ', #indexes, 'indexes')
