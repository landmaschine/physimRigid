
local spawn_count = 0

function on_init()
  local cam = scene:create("MainCamera")
  cam:set_camera(2.0, true)

  local ground = scene:create("Ground")
  ground:set_position(0, -1.5)
  ground:set_sprite(0.3, 0.3, 0.3, 1.0,  8.0, 0.3)
  ground:set_rigidbody({ is_static = true })
  ground:set_box_collider(4.0, 0.15)

  local lwall = scene:create("LeftWall")
  lwall:set_position(-3.5, 0)
  lwall:set_sprite(0.3, 0.3, 0.3, 1.0,  0.3, 5.0)
  lwall:set_rigidbody({ is_static = true })
  lwall:set_box_collider(0.15, 2.5)

  local rwall = scene:create("RightWall")
  rwall:set_position(3.5, 0)
  rwall:set_sprite(0.3, 0.3, 0.3, 1.0,  0.3, 5.0)
  rwall:set_rigidbody({ is_static = true })
  rwall:set_box_collider(0.15, 2.5)

  local ramp = scene:create("Ramp")
  ramp:set_position(-1.0, -0.5)
  ramp:set_rotation(-0.35)
  ramp:set_sprite(0.45, 0.45, 0.45, 1.0,  2.0, 0.08)
  ramp:set_rigidbody({ is_static = true })
  ramp:set_box_collider(1.0, 0.04)

  local plat = scene:create("Platform")
  plat:set_position(1.5, 0.0)
  plat:set_rotation(0.2)
  plat:set_sprite(0.45, 0.45, 0.45, 1.0,  1.2, 0.08)
  plat:set_rigidbody({ is_static = true })
  plat:set_box_collider(0.6, 0.04)

  local ball1 = scene:create("RedBall")
  ball1:set_position(-0.5, 1.0)
  ball1:set_sprite(0.9, 0.2, 0.2, 1.0,  0.3, 0.3)
  ball1:set_rigidbody({ mass = 0.2, restitution = 0.7 })
  ball1:set_circle_collider(0.15)

  local ball2 = scene:create("BlueBall")
  ball2:set_position(0.3, 1.5)
  ball2:set_sprite(0.2, 0.4, 0.95, 1.0,  0.24, 0.24)
  ball2:set_rigidbody({ mass = 0.1, restitution = 0.5 })
  ball2:set_circle_collider(0.12)

  local box1 = scene:create("OrangeBox")
  box1:set_position(-1.2, 1.4)
  box1:set_rotation(0.5)
  box1:set_sprite(1.0, 0.6, 0.1, 1.0,  0.3, 0.3)
  box1:set_rigidbody({ mass = 0.3, restitution = 0.3, friction = 0.5 })
  box1:set_box_collider(0.15, 0.15)

  local box2 = scene:create("PurpleBox")
  box2:set_position(0, 0.6)
  box2:set_sprite(0.6, 0.2, 0.8, 1.0,  0.5, 0.16)
  box2:set_rigidbody({ mass = 0.15, restitution = 0.4, friction = 0.4 })
  box2:set_box_collider(0.25, 0.08)

  local tri1 = scene:create("CyanTriangle")
  tri1:set_position(-0.8, 1.8)
  tri1:set_sprite(0.0, 0.9, 0.9, 1.0,  0.3, 0.26)
  tri1:set_rigidbody({ mass = 0.15, restitution = 0.4 })
  tri1:set_polygon_collider({
    {-0.15, -0.087},
    { 0.15, -0.087},
    { 0.0,   0.17}
  })

  local tri2 = scene:create("YellowTriangle")
  tri2:set_position(1.2, 1.2)
  tri2:set_rotation(1.0)
  tri2:set_sprite(1.0, 0.9, 0.1, 1.0,  0.26, 0.22)
  tri2:set_rigidbody({ mass = 0.1, restitution = 0.5 })
  tri2:set_polygon_collider({
    {-0.13, -0.075},
    { 0.13, -0.075},
    { 0.0,   0.15}
  })

  local pent = scene:create("GreenPentagon")
  pent:set_position(0.6, 1.8)
  pent:set_sprite(0.2, 0.85, 0.3, 1.0,  0.3, 0.3)
  pent:set_rigidbody({ mass = 0.2, restitution = 0.3 })
  pent:set_regular_polygon(5, 0.15)

  local hex = scene:create("PinkHexagon")
  hex:set_position(-1.6, 1.6)
  hex:set_sprite(0.95, 0.4, 0.6, 1.0,  0.36, 0.36)
  hex:set_rigidbody({ mass = 0.25, restitution = 0.2, friction = 0.6 })
  hex:set_regular_polygon(6, 0.18)

  for i = 0, 3 do
    local s = scene:create("StackBox" .. i)
    s:set_position(2.2, -1.1 + i * 0.26)
    s:set_sprite(0.5 + i * 0.1, 0.3, 0.8 - i * 0.1, 1.0,  0.24, 0.24)
    s:set_rigidbody({ mass = 0.1, restitution = 0.1, friction = 0.6 })
    s:set_box_collider(0.12, 0.12)
  end
end

function spawn_random(x, y)
  spawn_count = spawn_count + 1
  local name = "Spawned" .. spawn_count
  local e = scene:create(name)
  e:set_position(x, y)

  local kind = math.random(1, 4) 

  if kind == 1 then
    local r = 0.06 + math.random() * 0.1
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  r*2, r*2)
    e:set_rigidbody({ mass = r * 2, restitution = 0.3 })
    e:set_circle_collider(r)
  elseif kind == 2 then
    local w = 0.08 + math.random() * 0.18
    local h = 0.08 + math.random() * 0.18
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  w, h)
    e:set_rotation(math.random() * 3.14)
    e:set_rigidbody({ mass = w * h * 10, restitution = 0.2, friction = 0.5 })
    e:set_box_collider(w / 2, h / 2)
  elseif kind == 3 then
    
    local s = 0.08 + math.random() * 0.12
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  s*2, s*1.73)
    e:set_rotation(math.random() * 3.14)
    e:set_rigidbody({ mass = s * s * 5, restitution = 0.2 })
    e:set_polygon_collider({
      {-s, -s*0.58},
      { s, -s*0.58},
      { 0,  s*0.58}
    })
  else
    
    local sides = math.random(3, 8)
    local r = 0.06 + math.random() * 0.1
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  r*2, r*2)
    e:set_rotation(math.random() * 3.14)
    e:set_rigidbody({ mass = r * r * sides * 2, restitution = 0.2 })
    e:set_regular_polygon(sides, r)
  end
end

function on_update(dt)
  if input:is_key_pressed(KEY_SPACE) then
    local m = input.mouse_world
    spawn_random(m.x, m.y)
  end
end
