import os

from SCons.Script import Import


Import("env")

required_variables = (
    "PATUKA_FIREBASE_DATABASE_URL",
    "PATUKA_FIREBASE_API_KEY",
    "PATUKA_FIREBASE_USER_EMAIL",
    "PATUKA_FIREBASE_USER_PASSWORD",
    "PATUKA_WIFI_SSID",
    "PATUKA_WIFI_PASSWORD",
    "PATUKA_AP_SSID",
    "PATUKA_AP_PASSWORD",
)

missing_variables = [
    name for name in required_variables if not os.environ.get(name)
]

if missing_variables:
    raise RuntimeError(
        "Missing required environment variables: " + ", ".join(missing_variables)
    )

env.Append(
    CPPDEFINES=[(name, os.environ[name]) for name in required_variables]
)