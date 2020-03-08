
# Import Build Environment 
Import("env")

####################################################################################################

# Callback function to skip and ignore file from a build process
def skip_file_from_build(node):
    '''Skip and ignore file from a build process.'''
    return None

####################################################################################################

# Get used PIO Framework (Doesn't exists in Native)
build_framework = []
if "PIOFRAMEWORK" in env:
    build_framework = env["PIOFRAMEWORK"]
    print("Build framework - {}".format(build_framework))

# Check build and ignore custom mbedtls for ESP32 (To avoid conflict with esp-idf mbedtls component)
if ("arduino" in build_framework) or ("espidf" in build_framework):
    print("ESP32 Build detected, ignoring multihttpsclient/mbedtls.")
    env.AddBuildMiddleware(skip_file_from_build, "*multihttpsclient/mbedtls/*")
else:
    print("Generic Native Build detected, using src/utility/multihttpsclient/mbedtls.")
