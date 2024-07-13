#ifndef HEALTHCOMPONENT_H
#define HEALTHCOMPONENT_H


struct HealthComponent {
    int healthPercentage;

    HealthComponent (int healthComponent = 0) {
        this->healthPercentage = healthComponent;
    }
};

#endif