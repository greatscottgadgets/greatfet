pipeline {
    agent { 
        dockerfile {
            args '--group-add=46 --privileged -v /dev/bus/usb:/dev/bus/usb'
        }
    }
    environment {
        GIT_COMMITER_NAME = 'CI Person'
        GIT_COMMITER_EMAIL = 'ci@greatscottgadgets.com'
    }
    stages {
        stage('build') {
            steps {
                sh '''#!/bin/bash
                    echo "usbhub id:"
                    usbhub id
                    echo "usbhub power state:"
                    usbhub power state
                    echo "cycling usbhub greatfet port power"
                    usbhub power state --port 1 --reset
                    echo "INSTALLING libgreat/host ****************************************************************************************************************"
                    pushd libgreat/host/
                    python3 setup.py build
                    python3 setup.py install --user
                    popd
                    echo "INSTALLING host *************************************************************************************************************************"
                    pushd host/
                    python3 setup.py build
                    python3 setup.py install --user
                    popd
                    greatfet_info
                    echo "MAKING firmware *************************************************************************************************************************"
                    cd firmware/libopencm3
                    make clean
                    make
                    cd ../greatfet_usb
                    mkdir build
                    cd build
                    cmake ..
                    make
                    echo "DFUing firmware   ***********************************************************************************************************************"
                    greatfet_firmware --volatile-upload greatfet_usb.bin
                    echo "Sleep for 1s.."
                    sleep 1s
                    echo "FLASHING firmware ***********************************************************************************************************************"
                    greatfet_info
                    greatfet_firmware --write greatfet_usb.bin --reset
                    echo "Sleep for 1s.."
                    sleep 1s
                    greatfet_info
                    cd ../../..
                '''
            }
        }
    }
    post {
        always {
            echo 'One way or another, I have finished'
        }
    }
}