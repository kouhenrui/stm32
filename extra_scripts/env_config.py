"""从 .env / 系统环境变量读取敏感与动态配置，编译时注入固件。"""

Import("env")

import os
from pathlib import Path

DEFAULTS = {
    # 天气
    "WEATHER_CITY": "Shanghai,cn",
    "WEATHER_API_HOST": "api.openweathermap.org",
    "WEATHER_API_PORT": "80",
    "WEATHER_REFRESH_MS": "600000",
    "WEATHER_RETRY_MS": "60000",
    "WEATHER_HTTP_TIMEOUT_MS": "15000",
    # WiFi（最多 3 组，空或 your-* 占位符会被跳过）
    "WIFI_SSID_1": "your-ssid",
    "WIFI_PASSWORD_1": "your-password",
    "WIFI_SSID_2": "your-ssid-2",
    "WIFI_PASSWORD_2": "your-password-2",
    "WIFI_SSID_3": "your-ssid-3",
    "WIFI_PASSWORD_3": "your-password-3",
    # 功能开关 0/1
    "FEATURE_BUZZER": "0",
    "FEATURE_ESP8266": "1",
    "FEATURE_WEATHER": "1",
    # ESP8266
    "ESP_AT_TIMEOUT_MS": "3000",
    "ESP_WIFI_RETRY_MAX": "3",
}


def load_dotenv(project_dir):
    env_path = Path(project_dir) / ".env"
    if not env_path.is_file():
        return

    for raw in env_path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key and key not in os.environ:
            os.environ[key] = value


def c_string_literal(value):
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'\\"{escaped}\\"'


def env_str(name):
    default = DEFAULTS.get(name, "")
    return os.environ.get(name, default).strip() or default


def env_int(name):
    raw = env_str(name)
    try:
        return int(raw)
    except ValueError:
        return int(DEFAULTS.get(name, "0"))


def env_flag(name):
    return 1 if env_int(name) else 0


project_dir = env["PROJECT_DIR"]
load_dotenv(project_dir)

# --- 天气 ---
api_key = os.environ.get("WEATHER_API_KEY", "").strip()
city = env_str("WEATHER_CITY")
api_host = env_str("WEATHER_API_HOST")
api_port = env_int("WEATHER_API_PORT")
refresh_ms = env_int("WEATHER_REFRESH_MS")
retry_ms = env_int("WEATHER_RETRY_MS")
http_timeout_ms = env_int("WEATHER_HTTP_TIMEOUT_MS")

# --- WiFi ---
wifi_pairs = []
for i in (1, 2, 3):
    ssid = env_str(f"WIFI_SSID_{i}")
    password = env_str(f"WIFI_PASSWORD_{i}")
    wifi_pairs.append((ssid, password))

# --- 功能开关 ---
feature_buzzer = env_flag("FEATURE_BUZZER")
feature_esp8266 = env_flag("FEATURE_ESP8266")
feature_weather = env_flag("FEATURE_WEATHER")
esp_at_timeout = env_int("ESP_AT_TIMEOUT_MS")
esp_wifi_retry = env_int("ESP_WIFI_RETRY_MAX")

if feature_weather and not feature_esp8266:
    print("ERROR: FEATURE_WEATHER=1 需要 FEATURE_ESP8266=1")
    env.Exit(1)

if not api_key or api_key.startswith("your-api-key"):
    print("")
    print("=" * 60)
    print("WARNING: WEATHER_API_KEY 未设置")
    print("  请复制 .env.example 为 .env 并填入 API Key")
    print("  编译后 OLED 将显示 NO API KEY")
    print("=" * 60)
    print("")

configured_wifi = [
    ssid for ssid, _ in wifi_pairs
    if ssid and not ssid.startswith("your-ssid")
]
if feature_esp8266 and not configured_wifi:
    print("")
    print("=" * 60)
    print("WARNING: 未配置有效 WiFi（WIFI_SSID_1 等仍为占位符）")
    print("  请在 .env 中设置 WIFI_SSID_1 / WIFI_PASSWORD_1")
    print("=" * 60)
    print("")

cppdefines = [
    ("WEATHER_API_KEY", c_string_literal(api_key)),
    ("WEATHER_CITY", c_string_literal(city)),
    ("WEATHER_API_HOST", c_string_literal(api_host)),
    ("WEATHER_API_PORT", api_port),
    ("WEATHER_REFRESH_MS", refresh_ms),
    ("WEATHER_RETRY_MS", retry_ms),
    ("WEATHER_HTTP_TIMEOUT_MS", http_timeout_ms),
    ("FEATURE_BUZZER", feature_buzzer),
    ("FEATURE_ESP8266", feature_esp8266),
    ("FEATURE_WEATHER", feature_weather),
    ("ESP_AT_TIMEOUT_MS", esp_at_timeout),
    ("ESP_WIFI_RETRY_MAX", esp_wifi_retry),
]

for i, (ssid, password) in enumerate(wifi_pairs, start=1):
    cppdefines.append((f"WIFI_SSID_{i}", c_string_literal(ssid)))
    cppdefines.append((f"WIFI_PASSWORD_{i}", c_string_literal(password)))

env.Append(CPPDEFINES=cppdefines)

print("env_config:")
print(f"  WEATHER_CITY={city}")
print(f"  WEATHER_API_HOST={api_host}:{api_port}")
print(f"  WEATHER_API_KEY={'*' * 8 if api_key else '(empty)'}")
print(f"  FEATURES buzzer={feature_buzzer} esp8266={feature_esp8266} weather={feature_weather}")
print(f"  WIFI profiles configured: {len(configured_wifi)}")
