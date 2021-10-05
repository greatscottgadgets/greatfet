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
        stage('hub-check') {
            steps {
                sh '''#!/bin/bash
                    echo "usbhub id:"
                    usbhub id
                    echo "usbhub power state:"
                    usbhub power state
                '''
            }
        }
        stage('host-install') {
            steps {
                sh '''#!/bin/bash
                    git submodule init
                    git submodule update                    
                    python3 -m venv testing-venv
                    source testing-venv/bin/activate
                    pip install pyyaml
                    pushd libgreat/host/
                    python3 setup.py build
                    python3 setup.py install
                    popd
                    pushd host/
                    python3 setup.py build
                    python3 setup.py install
                    popd
                    attempts=0
                    while
                        usbhub power state --port 1 --reset
                        sleep 1s
                        greatfet_info
                        EXIT_CODE="$?"
                        attempts=$(( $attempts + 1 ))
                        [[ "$EXIT_CODE" != "19" && attempts < 3 ]]
                    do true; done
                    deactivate
                    if [ "$EXIT_CODE" == "19" ]
                    then
                        echo "Host tool installation success! Exiting.."
                        exit 0
                    elif [ "$EXIT_CODE" == "0" ]
                    then
                        echo "Failed to boot GreatFET into DFU mode! Check DFU pin jumper. Exiting.."
                        exit 99
                    elif [ "$EXIT_CODE" == "127" ]
                    then
                        echo "Host tool installation failed! Exiting.."
                        exit $EXIT_CODE
                    else
                        echo "god have mercy on your soul"
                        exit $EXIT_CODE
                    fi
                '''
            }
        }
        stage('firmware-install') {
            steps {
                sh '''#!/bin/bash
                    source testing-venv/bin/activate
                    cd firmware/libopencm3
                    make clean
                    make
                    cd ../greatfet_usb
                    mkdir build
                    cd build
                    cmake ..
                    make
                    cd ../../..
                    attempts=0
                    while
                        usbhub power state --port 1 --reset
                        sleep 1s
                        greatfet_firmware --volatile-upload firmware/greatfet_usb/build/greatfet_usb.bin
                        EXIT_CODE="$?"
                        attempts=$(( $attempts + 1 ))
                        [[ "$EXIT_CODE" != "0" && attempts < 3 ]]
                    do true; done
                    deactivate
                    if [ "$EXIT_CODE" == "0" ]
                    then
                        echo "DFU installation success! Exiting.."
                        exit $EXIT_CODE
                    elif [ "$EXIT_CODE" == "19" ]
                    then
                        echo "No GreatFET found! Disconnected? Exiting.."
                        exit 1
                    elif [ "$EXIT_CODE" == "127" ]
                    then
                        echo "Host tool installation failed! Exiting.."
                        exit $EXIT_CODE
                    else
                        echo "god have mercy on your soul"
                        exit $EXIT_CODE
                    fi
                '''
            }
        }
        stage('test') {
            steps {
                sh '''#!/bin/bash
                    source testing-venv/bin/activate
                    sleep 1s
                    greatfet_firmware --write firmware/greatfet_usb/build/greatfet_usb.bin
                    EXIT_CODE="$?"
                    if [ "$EXIT_CODE" == "1" ]
                    then
                        echo "No GreatFET found! Disconnected? Exiting.."
                        exit $EXIT_CODE
                    elif [ "$EXIT_CODE" == "0" ]
                    then
                        echo "Firmware successfully flashed!"
                    elif [ "$EXIT_CODE" == "127" ]
                    then
                        echo "Host tool installation failed! Exiting.."
                        exit $EXIT_CODE
                    else
                        echo "god have mercy on your soul"
                        exit $EXIT_CODE
                    fi
                    greatfet_info
                    deactivate
                    rm -rf testing-venv/
                '''
            }
        }
    }
    post {
        always {
            echo 'One way or another, I have finished'
            
            // Clean after build
            cleanWs(cleanWhenNotBuilt: false,
                    deleteDirs: true,
                    disableDeferredWipeout: true,
                    notFailBuild: true)
        }
    }
}
