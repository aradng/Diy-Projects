from zeroconf import Zeroconf
from time import sleep
import requests
import numpy as np

zeroconf = Zeroconf()


def get_ip(name="ESP-ColorPicker", service="_http._tcp.local."):
    try:
        return zeroconf.get_service_info(
            service, f"{name}.{service}"
        ).parsed_addresses()
    except (AssertionError, AttributeError):
        return []
    except KeyboardInterrupt:
        raise KeyboardInterrupt


def set_rgb(ip, rgb):
    try:
        print(rgb)
        r = requests.get(
            f"http://{ip}/rgb?r={rgb[0]}&g={rgb[1]}&b={rgb[2]}", timeout=2
        )
        # print(r.elapsed.total_seconds()*1000)
    except KeyboardInterrupt:
        raise KeyboardInterrupt
    except requests.exceptions.RequestException:
        raise TimeoutError
    except:
        pass


def fade_rgb(ip, rgb_start, rgb_end, fade_time=250):
    try:
        print(rgb_start, rgb_end, fade_time)
        r = requests.get(
            f"http://{ip}/fade?rs={rgb_start[0]}&re={rgb_end[0]}&gs={rgb_start[1]}&ge={rgb_end[1]}&bs={rgb_start[2]}&be={rgb_end[2]}&t={fade_time}",
            timeout=fade_time * 1.5 / 1000,
        )
        print(r.elapsed.total_seconds() * 1000)
    except KeyboardInterrupt:
        raise KeyboardInterrupt
    except requests.exceptions.RequestException:
        raise TimeoutError
    except:
        pass


while True:
    try:
        ip = get_ip()
        ip = "192.168.110.107"
        if not ip:
            continue
        ip = ip[0]
        ip = "192.168.110.107"
        print(f"found ESP at IP {ip}")
        print(f"running light show")
        # rgb = [0, 0, 255]
        # for i in range(3):
        #     set_rgb(ip, rgb)
        #     rgb.pop(0)
        #     rgb.append(0)
        #     sleep(1)
        # set_rgb(ip, [255, 255, 255])
        # sleep(1)
        rgb = [0, 0, 99]
        for i in range(3):
            fade_rgb(ip, [0, 0, 0], rgb, 1000)
            rgb.pop(0)
            rgb.append(0)
        rgb = [0, 0, 99]
        for i in range(3):
            fade_rgb(ip, [0, 0, 0], rgb, 1000)
            rgb.pop(0)
            rgb.append(255)
        set_rgb(ip, [0, 0, 0])
        sleep(3)
    except KeyboardInterrupt:
        break
    except:
        continue

zeroconf.close()
