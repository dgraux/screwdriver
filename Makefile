docclean : 	
		rm -rf documentation/drivers
		rm -rf documentation/software

doc: 	docclean
	doxygen documentation/doxy_drivers.conf
	doxygen documentation/doxy_software.conf
