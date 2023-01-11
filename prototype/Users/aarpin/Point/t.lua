require 'dump'



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


t={}

local kk=nextPointerFun()


print(kk())
dump(kk(t,9))
print(kk())
dump(kk(t,7))
print(kk())
dump(kk(t,6))
print(kk())
dump(kk(t,9))
print(kk())
kk(kk(t,9),'xxx')
print(kk())
dump(t)