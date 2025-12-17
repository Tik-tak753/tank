#ifndef HEALTHSYSTEM_H
#define HEALTHSYSTEM_H

/*
 * HealthSystem відстежує HP та кількість життів для сутності.
 */
class HealthSystem
{
public:
    void setMaxHealth(int hp) { m_health = hp; m_maxHealth = hp; }
    void setLives(int lives) { m_lives = lives; }

    int health() const { return m_health; }
    int lives() const { return m_lives; }

    bool isAlive() const { return m_health > 0 && m_lives > 0; }

    void takeDamage(int dmg);
    void restoreFullHealth();

private:
    int m_health = 1;
    int m_maxHealth = 1;
    int m_lives = 1;
};

#endif // HEALTHSYSTEM_H
