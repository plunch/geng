local o = scene.newobject()
o:set_sprite(6)
o:enable_physics(0.1, 70)
o:set_name('killme')
local c = o:add_box_collider(70, 70)
c:set_friction(12)
c:set_elasticity(0.2)
return o