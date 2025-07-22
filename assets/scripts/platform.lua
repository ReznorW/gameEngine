local direction = 1

function onStart()
end

function update(dt)
	local x, y, z = self:getPosition()

	if x >= 2 then
		direction = -1
	elseif x <= -2 then
		direction = 1
	end

	self:move(direction, 0, 0)
end