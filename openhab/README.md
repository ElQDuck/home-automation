# OpenHab Setup on an Raspberry Pi
The used Hardware is a Raspberry Pi 4 8GB

## Used Software
- [MQTT Explorer](http://mqtt-explorer.com/)
- [Lens](https://k8slens.dev/)

## Install Kubernetes
The used Kubernetes is [MicroK8s](https://github.com/canonical/microk8s)

### Install additional modules:
- storage
```bash
microk8s enable storage
```


### Get Access to MicroK8s
1. Get the kubeconfig file from MicroK8s
2. Use the kubeconfig in lens:
    1. ...

### Deploy Mosquitto to MicroK8s


### Config openHAB
#### Install Bindings
- MQTT Binding
- WLED Binding

# Troubleshooting
### Error while executing 'microk8s enable helm3' or '/snap/bin/microk8s enable helm3'
type the following:
```bash
export LC_ALL=C.UTF-8
export LANG=C.UTF-8
```
try again:
```bash
microk8s enable helm3
```