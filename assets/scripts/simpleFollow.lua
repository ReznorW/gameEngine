function onStart()
    targetName = getPlayerName()
    selfName = getName()
	followSpeed = 2
end

function update(dt)
	moveToward(selfName, targetName, followSpeed, dt)
    lookAt(selfName, targetName)
end