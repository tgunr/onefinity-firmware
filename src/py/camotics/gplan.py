class PlannerConfig(object):
    def __init__(self):
        self.max_velocity = 1000
        self.max_acceleration = 1000
        self.junction_acceleration = 100
        self.max_jerk = 100
        self.max_blend_error = 0.1
        self.search_velocity = 100
        self.probe_velocity = 50
        self.step_smoothing = 0.1
        self.min_time = 0.1

class Planner(object):
    def __init__(self, config):
        self.config = config
        self.position = [0, 0, 0, 0]  # X, Y, Z, A
        self.feed = 0
        self.speed = 0
        self.active = True

    def set_position(self, position):
        self.position = position

    def get_position(self):
        return self.position

    def set_feed(self, feed):
        self.feed = feed

    def set_speed(self, speed):
        self.speed = speed

    def load_machine(self, machine_var):
        pass  # Mock implementation

    def set_active(self, active):
        self.active = active

    def is_running(self):
        return False

    def restart(self):
        pass

    def stop(self):
        pass

    def reset(self, stop=True):
        if stop:
            self.stop()
        self.position = [0, 0, 0, 0]
        self.feed = 0
        self.speed = 0
        self.active = True

    def mdi(self, mdi):
        pass

    def load(self, path):
        return True  # Mock successful load

    def has_more(self):
        return False

    def next(self):
        return None

    def get_plan_time(self):
        return 0

    def synchronize(self, timeout):
        pass