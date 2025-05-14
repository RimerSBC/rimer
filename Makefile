.PHONY: clean All

All:
	@echo "==========Building project:[ rimer - Release_uf2 ]----------"
	@"$(MAKE)" -f  "rimer.mk" && "$(MAKE)" -f  "rimer.mk" PostBuild
clean:
	@echo "==========Cleaning project:[ rimer - Release_uf2 ]----------"
	@"$(MAKE)" -f  "rimer.mk" clean
