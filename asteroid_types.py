class Poly():
    def __init__(self):
        self.verts = []

class Asteroid():
    def __init__(self):
        self.health = 1
        self.polygons = []

    def polygon(self, verts):
        p = Poly()
        p.verts = verts
        self.polygons.append(p)

asteroids = []

a = Asteroid()
a.health = 2
a.polygon([(-1.5, -1.0), (1.0, -1.0), (2.5, 0.5), (1.0, 2.0), (-0.5, 1.0)])
asteroids.append(a)

a = Asteroid()
a.health = 1
a.polygon([(-1.5, -1.0), (-0.5, 1.0), (-1.0, 2.0), (-2.0, 1.0), (-2.0, 0.0)])
asteroids.append(a)

a = Asteroid()
a.health = 2
a.polygon([(-2.0, -2.0), (2.0, -2.0), (2.0, 2.0), (-2.0, 2.0)])
asteroids.append(a)

a = Asteroid()
a.health = 3
a.polygon([(-4.0, 0.0), (-2.0, -2.0), (1.0, -2.0), (5.0, 1.0), (4.0, 2.0), (-3.0, 2.0)])
asteroids.append(a)


print len(asteroids)
for a in asteroids:
    print a.health, len(a.polygons)
    for p in a.polygons:
        print len(p.verts)
        for v in p.verts:
            print v[0], v[1]

