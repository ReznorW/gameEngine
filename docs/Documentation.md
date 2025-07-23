# **Documentation**

## **Function Headers**

### **Object**

---

- #### Contructors

    - [`Object Object(String objName, String modelName, String textureName, String shaderName, String scriptName)`](#object-objectstring-objname-string-modelname-string-texturename-string-shadername-string-scriptname)
    - [`Object Object(String objName, String modelName, String textureName, String shaderName, String scriptName, String targetName)`](#object-objectstring-objname-string-modelname-string-texturename-string-shadername-string-scriptname-string-targetobject)

- #### Destructors

    - [`void destroy()`](#void-destroy)

- #### Getters

    - [`Number[3] getPosition()`](#number3-getposition)
    - [`Number[3] getRotation()`](#number3-getrotation)
    - [`Number[3] getScale()`](#number3-getscale)
    - [`String getName()`](#string-getname)

- #### Setters

    - [`void setPosition(Number x, Number y, Number z)`](#void-setpositionnumber-x-number-y-number-z)
    - [`void setRotation(Number pitch, Number yaw, Number roll)`](#void-setrotationnumber-pitch-number-yaw-number-roll)
    - [`void setScale(Number x, Number y, Number z)`](#void-setscalenumber-x-number-y-number-z)
    - [`void setScale(Number scale)`](#void-setscalenumber-scale)

- #### Physics

    - [`void move(Number x, Number y, Number z)`](#void-movenumber-x-number-y-number-z)
    - [`void rotate(Number pitch, Number yaw, Number roll)`](#void-rotatenumber-pitch-number-yaw-number-roll)
    - [`Bool checkCollision(String objName)`](#bool-checkcollisionstring-objname)
    - [`Bool checkCollison(Object obj)`](#bool-checkcollisionobject-obj)

- #### Trackers

    - [`void moveToward(String objName, Number speed)`](#void-movetowardstring-objname-number-speed)
    - [`void moveToward(Object obj, Number speed)`](#void-movetowardobject-obj-number-speed)
    - [`void lookAt(String objName)`](#void-lookatstring-objname)
    - [`void lookAt(Object obj)`](#void-lookatobject-obj)

### **Player**

---

- [`Object getPlayer()`](#object-getplayer)
- [`String getPlayerName()`](#string-getplayername)
- [`Number getPlayerSpeed()`](#number-getplayerspeed)
- [`Number getPlayerJump()`](#number-getplayerjump)
- [`void setPlayerSpeed(Number speed)`](#void-setplayerspeednumber-speed)
- [`void setPlayerJump(Number jump)`](#void-setplayerjumpnumber-jump)

### **Scene**

---

- [`void createObject(String objName, String modelName, String textureName, String shaderName, String scriptName)`](#void-createobjectstring-objname-string-modelname-string-texturename-string-shadername-string-scriptname)
- [`void createObject(String objName, String modelName, String textureName, String shaderName, String scriptName, String targetName)`](#void-createobjectstring-objname-string-modelname-string-texturename-string-shadername-string-scriptname-string-targetName)
- [`void destroyObject(String objName)`](#void-destroyobjectstring-objname)
- [`void destroyObject(Object obj)`](#void-destroyobjectobject-obj)
- [`Object getObject(String objName)`](#object-getobjectstring-objname)
- [`Number[4] getSkyColor()`](#number4-getskycolor)
- [`Number[3] getGravity()`](#number3-getgravity)
- [`Number getDrag()`](#number-getdrag)
- [`void setSkyColor(Number red, Number green, Number blue, Number alpha)`](#void-setskycolornumber-red-number-green-number-blue-number-alpha)
- [`void setGravity(Number y)`](#void-setgravitynumber-y)
- [`void setGravity(Number x, Number y, Number z)`](#void-setgravitynumber-x-number-y-number-z)
- [`void setDrag(Number drag)`](#void-setdragnumber-drag)

### **Input**

---

- [`Bool isKeyPressed(String key)`](#bool-iskeypressedstring-key)
- [`Bool isKeyPressedOnce(String key)`](#bool-iskeypressedoncestring-key)

### **Misc**

---

- [`Number rand(Number max)`](#number-randnumber-max)
- [`Number rand(Number min, Number max)`](#number-randnumber-min-number-max)

## **Function Definitions**

### **Object**

---

**Contructors**

---

#### `Object Object(String objName, String modelName, String textureName, String shaderName, String scriptName)`

Creates and returns a new Object with a specified name, model, texture, shader, and script.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName
- `String` scriptName

*Returns:* An `Object` instance.

*Example:*

    local obj = Object("enemy", "pyramid", "red.png", "default", "follow")

#### `Object Object(String objName, String modelName, String textureName, String shaderName, string scriptName, string targetObject)`

 Creates and returns a new Object with a specified name, model, texture, shader, and script at the targeted object's position.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName
- `String` scriptName
- `String` targetName

*Returns:*

- `Object` obj

*Example:*

    local obj = Object("enemy", "pyramid", "red.png", "default", "follow", "spawnObject")

**Destructors**

---

#### `void destroy()`

Destroys `this`.

*Example:*

    obj:destroy()

**Getters**

---

#### `Number[3] getPosition()`

Returns the x, y, and z positions of `this`.

*Returns:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    local x, y, z = obj:getPosition()

#### `Number[3] getRotation()`

Returns the degrees of pitch, yaw, and roll of `this`.

*Returns:*

- `Number` pitch 
- `Number` yaw
- `Number` roll

*Example:*

    local pitch, yaw, roll = obj:getRotation()

#### `Number[3] getScale()`

Returns the x, y, and z scales of `this`.

*Returns:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    local x, y, z = obj:getScale()

#### `String getName()`

Returns the name of `this`.

*Returns:*

- `String` name

*Example:*

    local objName = obj:getName()

**Setters**

---

#### `void setPosition(Number x, Number y, Number z)`

Sets the x, y, and z positions of `this` to specified values.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    obj:setPosition(1, 2, 3)

#### `void setRotation(Number pitch, Number yaw, Number roll)`

Sets the pitch, yaw, and roll of `this` to specified degrees.

*Parameters:*

- `Number` pitch
- `Number` yaw
- `Number` roll

*Example:*

    obj:setRotation(90, 180, 270)

#### `void setScale(Number x, Number y, Number z)`

Sets the x, y, and z scaling of `this` to specified values.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    obj:setScale(2, 3, 4)

#### `void setScale(Number scale)`

Sets the scaling of `this` to specified value.

*Parameters:*

- `Number` scale

*Example:*

    obj:setScale(2)

**Physics**

---

#### `void move(Number x, Number y, Number z)`

Adds velocity x, y, and z to `this`.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    obj:move(1, 2, 3)

#### `void rotate(Number pitch, Number yaw, Number roll)`

Adds pitch, yaw, and roll degrees to `this`'s rotation.

*Parameters:*

- `Number` pitch
- `Number` yaw
- `Number` roll

*Example:*

    obj:rotate(90, 180, 270)

#### `Bool checkCollision(String objName)`

Checks if `this` has collided with specific object.

*Parameters:*

- `String` objName

*Returns:*

- `Bool` isCollided

*Example:*

    obj:checkCollision("otherObjName")

#### `Bool checkCollision(Object obj)`

Checks if `this` has collided with specified object.

*Parameters:*

- `Object` obj

*Returns:*

- `Bool` isCollided

*Example:*

    obj:checkCollision(otherObj)

**Trackers**

---

#### `void moveToward(String objName, Number speed)`  

Applies speed to `this`'s velocity in the direction of specified object.

*Parameters:*

- `String` objName
- `Number` speed

*Example:*

    obj:moveToward("otherObjName", 2)

#### `void moveToward(Object obj, Number speed)`  

Applies speed to `this`'s velocity in the direction of specified object.

*Parameters:*

- `Object` obj
- `Number` speed

*Example:*

    obj:moveToward(otherObj, 2)

#### `void lookAt(String objName)`  

Sets `this`'s yaw to the direction of specified object.

*Parameters:*

- `String` objName

*Example:*

    obj:lookAt("otherObjName")

#### `void lookAt(Object obj)`  

Sets `this`'s yaw to the direction of specified object.

*Parameters:*

- `Object` obj

*Example:*

    obj:lookAt(otherObj)

### **Player**

---

#### `Object getPlayer()`  

Returns the player object.

*Returns:*

- `Object` playerObj

*Example:*

    local player = getPlayer()

#### `String getPlayerName()`  

Returns the player object's name.

*Returns:*

- `String` playerObjName

*Example:*

    local playerName = getPlayerName()

#### `Number getPlayerSpeed()`  

Returns the player object's speed.

*Returns:*

- `Number` playerObjSpeed

*Example:*

    local playerSpeed = getPlayerSpeed()

#### `Number getPlayerJump()`  

Returns the player object's jump strength.

*Returns:*

- `Number` playerObjJump

*Example:*

    local playerJump = getPlayerJump()

#### `void setPlayerSpeed(Number speed)`  

Sets the player object's speed.

*Parameters:*

- `Number` speed

*Example:*

    setPlayerSpeed(1)

#### `void setPlayerJump(Number jump)`  

Sets the player object's jump strength.

*Parameters:*

- `Number` jump

*Example:*

    setPlayerJump(10)

### **Scene**

---

#### `void createObject(String objName, String modelName, String textureName, String shaderName, String scriptName)` 

Creates a new Object with a specified name, model, texture, shader, and script.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName
- `String` scriptName

*Example:*

    createObject("crate", "cube", "wood.png", "default")

#### `void createObject(String objName, String modelName, String textureName, String shaderName, String scriptName, String targetName)`  

Creates a new Object with a specified name, model, texture, shader, and script at the targeted object's position.

*Parameters:*
- `String` objName
- `String` modelName
- `String` textureName
- `String` shaderName
- `String` scriptName
- `String` targetName

*Example:*

    createObject("enemy", "pyramid", "red.png", "default", "follow", "spawnObject")

#### `void destroyObject(String objName)`  

Destroys the specified object.

*Parameters:*

- `String` objName

*Example:*

    destroyObject("crate")

#### `void destroyObject(Object obj)`  

Destroys the specified object.

*Parameters:*

- `Object` obj

*Example:*

    destroyObject(crateObj)

#### `Object getObject(String objName)`  

Returns the specified object.

*Parameters:*

- `String` objName

*Returns:*

- `Object` obj

*Example:*

    local obj = getObject("enemy")

#### `Number[4] getSkyColor()`  

Returns the scene's sky color.

*Returns:*

- `Number` red
- `Number` green
- `Number` blue
- `Number` alpha

*Example:*

    local r, g, b, a = getSkyColor()

#### `Number[3] getGravity()`  

Returns the scene's gravity.

*Returns:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    local x, y, z = getGravity()

#### `Number getDrag()`  

Returns the scene's drag.

*Returns:*

- `Number` drag

*Example:*

    local drag = getDrag()

#### `void setSkyColor(Number red, Number green, Number blue, Number alpha)`  

Sets the scene's sky color. All parameters must be between 0 and 255.

*Parameters:*

- `Number` red
- `Number` green
- `Number` blue
- `Number` alpha

*Example:*

    setSkyColor(255, 0, 255, 255)

#### `void setGravity(Number y)`  

Sets the scene's y component of gravity.

*Parameters:*

- `Number` y

*Example:*

    setGravity(-9.8)

#### `void setGravity(Number x, Number y, Number z)`  

Sets the scene's gravity.

*Parameters:*

- `Number` x
- `Number` y
- `Number` z

*Example:*

    setGravity(0, -9.8, 0)

#### `void setDrag(Number drag)`  

Sets the scene's drag. Parameter must be between 0 and 1.

*Parameters:*

- `Number` drag

*Example:*

    setDrag(0.8)

### **Input**

---

#### `Bool isKeyPressed(String key)`  

Return whether or not the specified key is pressed down.

*Parameters:*

- `String` key

*Returns:*

- `Bool` isPressed

*Example:*

    if (isKeyPressed("W")) then
        obj:move(1, 0, 0)
    end

#### `Bool isKeyPressedOnce(String key)`  

Return whether or not the specified key is pressed once.

*Parameters:*

- `String` key

*Returns:*

- `Bool` isPressedOnce

*Example:*

    if (isKeyPressedOnce("X")) then
        createObject("enemy", "pyramid", "red.png", "default", "follow", "spawnObject")
    end

### **Misc**

---

#### `Number rand(Number max)`

Returns a random number between 0 and max.

*Parameters:*

- `Number` max

*Returns:*

- `Number` random

*Example:*

    local random = rand(10)

#### `Number rand(Number min, Number max)`

Returns a random number between min and max.

*Parameters:*

- `Number` min
- `Number` max

*Returns:*

- `Number` random

*Example:*

    local random = rand(5, 10)