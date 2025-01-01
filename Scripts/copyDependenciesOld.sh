#!/bin/bash

for lib in $(ldd ../build/bin/CalFluxNeutron | grep -oP '(?<= => ).*(?= \()'); do
    cp "$lib" /home/andreia-simas/Desktop/NeutronFluxCalculation/build/DEPENDENCIES/
done

