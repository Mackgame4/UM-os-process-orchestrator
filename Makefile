all: build

build:
	@echo "Building..."
	$(MAKE) build_client
	$(MAKE) build_orchestrator
	$(MAKE) compile_commands

client: build_client
orchestrator: build_orchestrator

build_client:
	@echo "Building client..."
	$(MAKE) -C client/

build_orchestrator:
	@echo "Building orchestrator..."
	$(MAKE) -C server/

compile_commands:
	@echo "Building command binaries..."
	$(MAKE) -C bin/

clean:
	@echo "Cleaning..."
	$(MAKE) -C client/ clean
	$(MAKE) -C server/ clean
	$(MAKE) -C bin/ clean
	@rm datapipe/c_fifo_* datapipe/taskoutput_*.txt datapipe/log.txt