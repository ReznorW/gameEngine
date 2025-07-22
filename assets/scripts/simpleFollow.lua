function onStart()
	followSpeed = 1
end

function update(dt)
	target = getObject(getPlayerName())
	self:moveToward(target, followSpeed, 2)
    self:lookAt(target)
end