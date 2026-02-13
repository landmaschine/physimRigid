-- scripts/init.lua
-- ─────────────────────────────────────────────────────────
-- Demo scene: circles, boxes, triangles, pentagons, hexagons.
-- Click & drag any dynamic body with the mouse.
-- Press SPACE to spawn a random shape at the cursor.
-- ─────────────────────────────────────────────────────────

local spawn_count = 0

function on_init()
  -- ── Camera ───────────────────────────────────────────
  local cam = scene:create("MainCamera")
  cam:set_camera(10.0, true)

  -- ── Static ground ────────────────────────────────────
  local ground = scene:create("Ground")
  ground:set_position(0, -6)
  ground:set_sprite(0.3, 0.3, 0.3, 1.0,  20.0, 1.0)
  ground:set_rigidbody({ is_static = true })
  ground:set_box_collider(10.0, 0.5)

  -- ── Left wall ────────────────────────────────────────
  local lwall = scene:create("LeftWall")
  lwall:set_position(-10, 0)
  lwall:set_sprite(0.3, 0.3, 0.3, 1.0,  1.0, 14.0)
  lwall:set_rigidbody({ is_static = true })
  lwall:set_box_collider(0.5, 7.0)

  -- ── Right wall ───────────────────────────────────────
  local rwall = scene:create("RightWall")
  rwall:set_position(10, 0)
  rwall:set_sprite(0.3, 0.3, 0.3, 1.0,  1.0, 14.0)
  rwall:set_rigidbody({ is_static = true })
  rwall:set_box_collider(0.5, 7.0)

  -- ── Angled ramp ──────────────────────────────────────
  local ramp = scene:create("Ramp")
  ramp:set_position(-4, -3)
  ramp:set_rotation(-0.35)
  ramp:set_sprite(0.45, 0.45, 0.45, 1.0,  6.0, 0.3)
  ramp:set_rigidbody({ is_static = true })
  ramp:set_box_collider(3.0, 0.15)

  -- ── Small platform ───────────────────────────────────
  local plat = scene:create("Platform")
  plat:set_position(5, -1)
  plat:set_rotation(0.2)
  plat:set_sprite(0.45, 0.45, 0.45, 1.0,  4.0, 0.3)
  plat:set_rigidbody({ is_static = true })
  plat:set_box_collider(2.0, 0.15)

  -- ── Dynamic circles ──────────────────────────────────
  local ball1 = scene:create("RedBall")
  ball1:set_position(-2, 5)
  ball1:set_sprite(0.9, 0.2, 0.2, 1.0,  1.0, 1.0)
  ball1:set_rigidbody({ mass = 1.0, restitution = 0.7 })
  ball1:set_circle_collider(0.5)

  local ball2 = scene:create("BlueBall")
  ball2:set_position(1, 7)
  ball2:set_sprite(0.2, 0.4, 0.95, 1.0,  0.8, 0.8)
  ball2:set_rigidbody({ mass = 0.6, restitution = 0.5 })
  ball2:set_circle_collider(0.4)

  -- ── Dynamic boxes ────────────────────────────────────
  local box1 = scene:create("OrangeBox")
  box1:set_position(-5, 6)
  box1:set_rotation(0.5)
  box1:set_sprite(1.0, 0.6, 0.1, 1.0,  1.2, 1.2)
  box1:set_rigidbody({ mass = 1.5, restitution = 0.3, friction = 0.5 })
  box1:set_box_collider(0.6, 0.6)

  local box2 = scene:create("PurpleBox")
  box2:set_position(0, 3)
  box2:set_sprite(0.6, 0.2, 0.8, 1.0,  1.8, 0.6)
  box2:set_rigidbody({ mass = 0.8, restitution = 0.4, friction = 0.4 })
  box2:set_box_collider(0.9, 0.3)

  -- ── Dynamic triangles ────────────────────────────────
  local tri1 = scene:create("CyanTriangle")
  tri1:set_position(-3, 8)
  tri1:set_sprite(0.0, 0.9, 0.9, 1.0,  1.2, 1.04)
  tri1:set_rigidbody({ mass = 0.8, restitution = 0.4 })
  tri1:set_polygon_collider({
    {-0.6, -0.3},
    { 0.6, -0.3},
    { 0.0,  0.52}
  })

  local tri2 = scene:create("YellowTriangle")
  tri2:set_position(4, 5)
  tri2:set_rotation(1.0)
  tri2:set_sprite(1.0, 0.9, 0.1, 1.0,  1.0, 0.87)
  tri2:set_rigidbody({ mass = 0.6, restitution = 0.5 })
  tri2:set_polygon_collider({
    {-0.5, -0.29},
    { 0.5, -0.29},
    { 0.0,  0.58}
  })

  -- ── Dynamic pentagon ─────────────────────────────────
  local pent = scene:create("GreenPentagon")
  pent:set_position(2, 9)
  pent:set_sprite(0.2, 0.85, 0.3, 1.0,  1.2, 1.2)
  pent:set_rigidbody({ mass = 1.2, restitution = 0.3 })
  pent:set_regular_polygon(5, 0.6)

  -- ── Dynamic hexagon ──────────────────────────────────
  local hex = scene:create("PinkHexagon")
  hex:set_position(-6, 7)
  hex:set_sprite(0.95, 0.4, 0.6, 1.0,  1.4, 1.4)
  hex:set_rigidbody({ mass = 1.5, restitution = 0.2, friction = 0.6 })
  hex:set_regular_polygon(6, 0.7)

  -- ── A stack of small boxes ───────────────────────────
  for i = 0, 3 do
    local s = scene:create("StackBox" .. i)
    s:set_position(7, -4.5 + i * 1.05)
    s:set_sprite(0.5 + i * 0.1, 0.3, 0.8 - i * 0.1, 1.0,  0.9, 0.9)
    s:set_rigidbody({ mass = 0.5, restitution = 0.1, friction = 0.6 })
    s:set_box_collider(0.45, 0.45)
  end
end

-- ── Helper: spawn a random shape at cursor ──────────────
function spawn_random(x, y)
  spawn_count = spawn_count + 1
  local name = "Spawned" .. spawn_count
  local e = scene:create(name)
  e:set_position(x, y)

  local kind = math.random(1, 4) -- 1=circle, 2=box, 3=triangle, 4=polygon

  if kind == 1 then
    -- circle
    local r = 0.2 + math.random() * 0.4
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  r*2, r*2)
    e:set_rigidbody({ mass = r * 2, restitution = 0.3 })
    e:set_circle_collider(r)
  elseif kind == 2 then
    -- box
    local w = 0.3 + math.random() * 0.7
    local h = 0.3 + math.random() * 0.7
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  w, h)
    e:set_rotation(math.random() * 3.14)
    e:set_rigidbody({ mass = w * h, restitution = 0.2, friction = 0.5 })
    e:set_box_collider(w / 2, h / 2)
  elseif kind == 3 then
    -- triangle
    local s = 0.3 + math.random() * 0.5
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  s*2, s*1.73)
    e:set_rotation(math.random() * 3.14)
    e:set_rigidbody({ mass = s * s, restitution = 0.2 })
    e:set_polygon_collider({
      {-s, -s*0.58},
      { s, -s*0.58},
      { 0,  s*0.58}
    })
  else
    -- random regular polygon (3-8 sides)
    local sides = math.random(3, 8)
    local r = 0.25 + math.random() * 0.4
    e:set_sprite(math.random(), math.random(), math.random(), 1.0,  r*2, r*2)
    e:set_rotation(math.random() * 3.14)
    e:set_rigidbody({ mass = r * r * sides * 0.3, restitution = 0.2 })
    e:set_regular_polygon(sides, r)
  end
end

function on_update(dt)
  -- Press SPACE to spawn a random body at the mouse cursor
  if input:is_key_pressed(KEY_SPACE) then
    local m = input.mouse_world
    spawn_random(m.x, m.y)
  end
end
