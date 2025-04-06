
#include "Angel.h"
#include "PhysicsObject.h"

    PhysicsObject::PhysicsObject(vec3 initPos, vec3 initVel, vec3 initAcc, float initMass)
    {
        PhysicsObject::position = initPos;
        PhysicsObject::velocity = initVel;
        PhysicsObject::acceleration = initAcc;
        PhysicsObject::mass = initMass;
    }


    void PhysicsObject::update(double deltaTime)
    {
        if (position.y <= -0.75f && velocity.y < 0.0f)
        {
            vec3 surface(0.0, 1.0, 0.0);
            bounce(surface);
        }

        vec3 gravity(0.0, -9.8, 0.0);
        vec3 gravitionalForce = gravity*mass;
        float resistence = 0.2;

        applyForce(gravitionalForce);
        applyResistence(resistence);
        position = position + 0.5*acceleration*deltaTime*deltaTime + velocity*deltaTime;
        velocity = velocity + acceleration * deltaTime;
        acceleration = vec3(0.0, 0.0, 0.0);
    }

    void PhysicsObject::applyForce(vec3 force)
    {
        acceleration += force/mass;
    }

    void PhysicsObject::bounce(vec3 surface_normal)
    {
        float restitution = 0.9f;
        vec3 normalized_surface_normal = normalize(surface_normal);
        velocity = velocity - (1.0f + restitution) * dot(velocity, normalized_surface_normal) * normalized_surface_normal;
    }

    void PhysicsObject::applyResistence(float resistenceCoefficient)
    {
        //apply force according to the direction of the velocity
        vec3 resistence_force = - resistenceCoefficient * velocity * length(velocity);
        applyForce(resistence_force);
    }
