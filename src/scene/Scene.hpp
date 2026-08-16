#pragma once

namespace solar::scene {

class Scene {
public:
    virtual ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

protected:
    Scene() = default;
};

} // namespace solar::scene

