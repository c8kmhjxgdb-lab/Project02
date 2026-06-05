#pragma once

#include <box2d/box2d.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <string>
#include <vector>

struct ScheduleEntry {
    float startTime;
    float endTime;
    glm::vec2 position;
    std::string action;
    std::string dialogueTrigger;
};

class NPC {
public:
    NPC(const std::string& name);
    virtual ~NPC();

    void createBody(b2WorldId world, const glm::vec2& pos);
    b2BodyId getBodyId() const { return bodyId; }

    virtual void update(float dt, float gameTime);

    virtual void render(const glm::mat4& viewProj);

    virtual bool canInteract(const glm::vec2& playerPos, float range) const;
    virtual void onInteract();

    void setSchedule(const std::vector<ScheduleEntry>& schedule);
    void setCurrentSchedule(const std::string& scheduleId);

    const std::string& getName() const { return name; }
    glm::vec2 getPosition() const;
    glm::vec3 getColor() const { return bodyColor; }
    void setColor(const glm::vec3& c) { bodyColor = c; }

    void setDialogueId(const std::string& id) { dialogueId = id; }
    const std::string& getDialogueId() const { return dialogueId; }

    bool hasBody() const { return b2Body_IsValid(bodyId); }

protected:
    std::string name;
    b2BodyId bodyId;
    glm::vec3 bodyColor;
    std::string dialogueId;

    std::vector<ScheduleEntry> currentSchedule;
    int currentScheduleIndex;

    enum class State {
        Idle, Walking
    } state;
    glm::vec2 targetPosition;

    int findScheduleForTime(float gameTime) const;
};
