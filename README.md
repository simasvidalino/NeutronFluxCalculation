# NeutronFluxCalculation
User interface for Neutron Flux Calculation


# Deploy the app
- cmake -S . -B build -DENABLE_DEPLOY=ON

- Change CMakeList

- To enable deploy:
  - Change `ENABLE_DEPLOY` to **<span style="color:green;">ON</span>**
- To enable desable:
  - Change `ENABLE_DEPLOY` to **<span style="color:red;">OFF</span>**

- To use the .appImage
    - The computer must be updated:
    sudo do-release-upgrade -d
    - Turn Neutron_Scalar_Flux_Calculator-x86_64.AppImage a executable: 
      chmod +x Neutron_Scalar_Flux_Calculator-x86_64.AppImage
    - Run: 
      ./Neutron_Scalar_Flux_Calculator-x86_64.AppImage


