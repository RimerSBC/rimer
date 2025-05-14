.PHONY: clean All

All:
	@echo "==========Building project:[ rimer - Debug ]----------"
	@"$(MAKE)" -f  "rimer.mk" && "$(MAKE)" -f  "rimer.mk" PostBuild
clean:
	@echo "==========Cleaning project:[ rimer - Debug ]----------"
	@"$(MAKE)" -f  "rimer.mk" clean
