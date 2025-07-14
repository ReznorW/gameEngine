function onStart()
	followSpeed = 2
end

function update(dt)
	target = getObject(getPlayerName())
	self:moveToward(target, followSpeed)
    self:lookAt(target)
end