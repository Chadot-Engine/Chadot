def can_build(env, platform):
    env.module_add_dependencies("lottie", ["svg"], False)
    return True


def configure(env):
    pass
