#!/bin/bash

mkdir -p build && cd build
cmake ..
if [ $? -ne 0 ]; then
  echo "Stop 1: CMake failed"
  exit 1
fi

make -j$(nproc)
if [ $? -ne 0 ]; then
  echo "Stop 2: Make failed"
  exit 2
fi

sudo mv gmacrod /usr/bin/gmacrod
if [ $? -ne 0 ]; then
  echo "Stop 3: Failed to move binary"
  exit 3
fi

sudo mkdir -p /etc/sv/gmacrod

sudo tee /etc/sv/gmacrod/run << EOF > /dev/null
#!/bin/sh
exec 2>&1
exec /usr/bin/gmacrod -c /home/${SUDO_USER:-$USER}/.config/gmacrod/
EOF

if [ $? -ne 0 ]; then
  echo "Stop 4: Failed to create run script"
  exit 4
fi

sudo chmod +x /etc/sv/gmacrod/run

sudo mkdir -p /etc/sv/gmacrod/log
sudo mkdir -p /var/log/gmacrod

sudo tee /etc/sv/gmacrod/log/run << 'EOF' > /dev/null
#!/bin/sh
exec svlogd -tt /var/log/gmacrod
EOF

if [ $? -ne 0 ]; then
  echo "Stop 5: Failed to create log script"
  exit 5
fi

sudo chmod +x /etc/sv/gmacrod/log/run

sudo ln -sf /etc/sv/gmacrod /var/service/
if [ $? -ne 0 ]; then
  echo "Stop 6: Failed to symlink service"
  exit 6
fi

echo "done!"