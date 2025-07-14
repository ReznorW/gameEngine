local counter

function onStart()
    counter = 1
end

function update(dt)
    if isKeyPressed("N") then
        local newObjectName = "New" .. counter
        createObject(newObjectName, "pyramid", "red.png", "default", "simpleFollow")
        counter = counter + 1
    end

	if isKeyPressed("M") then
		counter = counter - 1
		local deletedObj = "New" .. counter
		destroyObject(deletedObj)
	end
end