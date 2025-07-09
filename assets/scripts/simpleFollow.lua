local followSpeed = 1.0
local targetName = "player"
local selfName = "follower"

function update(dt)
    local tx, ty, tz = getPosition(targetName)
    local sx, sy, sz = getPosition(selfName)

    -- If target or self positions are invalid (i.e. object not found), exit early
    if not tx or not sx then
        return
    end

    local dx = tx - sx
    local dy = ty - sy
    local dz = tz - sz

    local distance = math.sqrt(dx * dx + dy * dy + dz * dz)
    if distance < 0.01 then
        return
    end

    local nx = dx / distance
    local ny = dy / distance
    local nz = dz / distance

    local moveDist = math.min(distance, followSpeed * dt)

    moveObject(selfName, nx * moveDist, ny * moveDist, nz * moveDist)
end