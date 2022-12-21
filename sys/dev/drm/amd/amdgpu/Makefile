#
# Makefile for the drm/amdgpu driver.
#

KMOD=	amdgpu

.PATH:	${.CURDIR}/../../scheduler \
	${.CURDIR}/../acp \
	${.CURDIR}/../pm \
	${.CURDIR}/../pm/powerplay \
	${.CURDIR}/../pm/powerplay/hwmgr \
	${.CURDIR}/../pm/powerplay/smumgr \
	${.CURDIR}/../powerplay \
	${.CURDIR}/../lib \
	${.CURDIR}/../display/amdgpu_dm \
	${.CURDIR}/../display/dc \
	${.CURDIR}/../display/dc/basics \
	${.CURDIR}/../display/dc/bios \
	${.CURDIR}/../display/dc/bios/dce60 \
	${.CURDIR}/../display/dc/bios/dce80 \
	${.CURDIR}/../display/dc/bios/dce110 \
	${.CURDIR}/../display/dc/bios/dce112 \
	${.CURDIR}/../display/dc/calcs \
	${.CURDIR}/../display/dc/core \
	${.CURDIR}/../display/dc/dce \
	${.CURDIR}/../display/dc/dce60 \
	${.CURDIR}/../display/dc/dsc \
	${.CURDIR}/../display/dc/gpio \
	${.CURDIR}/../display/dc/gpio/dce110 \
	${.CURDIR}/../display/dc/gpio/dce120 \
	${.CURDIR}/../display/dc/gpio/dce80 \
	${.CURDIR}/../display/dc/gpio/dce60 \
	${.CURDIR}/../display/dc/gpio/dcn10 \
	${.CURDIR}/../display/dc/gpio/dcn20 \
	${.CURDIR}/../display/dc/gpio/dcn21 \
	${.CURDIR}/../display/dc/gpio/dcn30 \
	${.CURDIR}/../display/dc/gpio/diagnostics \
	${.CURDIR}/../display/dc/i2caux \
	${.CURDIR}/../display/dc/i2caux/dce100 \
	${.CURDIR}/../display/dc/i2caux/dce110 \
	${.CURDIR}/../display/dc/i2caux/dce112 \
	${.CURDIR}/../display/dc/i2caux/dce120 \
	${.CURDIR}/../display/dc/i2caux/dce80 \
	${.CURDIR}/../display/dc/i2caux/dcn10 \
	${.CURDIR}/../display/dc/i2caux/diagnostics \
	${.CURDIR}/../display/dc/irq \
	${.CURDIR}/../display/dc/irq/dce110 \
	${.CURDIR}/../display/dc/irq/dce120 \
	${.CURDIR}/../display/dc/irq/dce80 \
	${.CURDIR}/../display/dc/irq/dce60 \
	${.CURDIR}/../display/dc/irq/dcn10 \
	${.CURDIR}/../display/dc/irq/dcn20 \
	${.CURDIR}/../display/dc/irq/dcn21 \
	${.CURDIR}/../display/dc/irq/dcn302 \
	${.CURDIR}/../display/dc/irq/dcn303 \
	${.CURDIR}/../display/dc/irq/dcn31 \
	${.CURDIR}/../display/dc/virtual \
	${.CURDIR}/../display/dc/dcn10 \
	${.CURDIR}/../display/dc/dcn20 \
	${.CURDIR}/../display/dc/dcn21 \
	${.CURDIR}/../display/dc/dcn30 \
	${.CURDIR}/../display/dc/dcn301 \
	${.CURDIR}/../display/dc/dcn302 \
	${.CURDIR}/../display/dc/dcn303 \
	${.CURDIR}/../display/dc/dcn31 \
	${.CURDIR}/../display/dc/clk_mgr \
	${.CURDIR}/../display/dc/clk_mgr/dce100 \
	${.CURDIR}/../display/dc/clk_mgr/dce110 \
	${.CURDIR}/../display/dc/clk_mgr/dce112 \
	${.CURDIR}/../display/dc/clk_mgr/dce120 \
	${.CURDIR}/../display/dc/clk_mgr/dce60 \
	${.CURDIR}/../display/dc/clk_mgr/dcn10 \
	${.CURDIR}/../display/dc/clk_mgr/dcn20 \
	${.CURDIR}/../display/dc/clk_mgr/dcn21 \
	${.CURDIR}/../display/dc/clk_mgr/dcn30 \
	${.CURDIR}/../display/dc/clk_mgr/dcn301 \
	${.CURDIR}/../display/dc/clk_mgr/dcn31 \
	${.CURDIR}/../display/dc/dml \
	${.CURDIR}/../display/dc/dml/dcn20 \
	${.CURDIR}/../display/dc/dml/dcn21 \
	${.CURDIR}/../display/dc/dml/dcn2x \
	${.CURDIR}/../display/dc/dml/dcn30 \
	${.CURDIR}/../display/dc/dml/dcn31 \
	${.CURDIR}/../display/dc/dml/dsc \
	${.CURDIR}/../display/dc/dce120 \
	${.CURDIR}/../display/dc/dce112 \
	${.CURDIR}/../display/dc/dce110 \
	${.CURDIR}/../display/dc/dce100 \
	${.CURDIR}/../display/dc/dce80 \
	${.CURDIR}/../display/dc/hdcp \
	${.CURDIR}/../display/dmub/src \
	${.CURDIR}/../display/modules/freesync \
	${.CURDIR}/../display/modules/color \
	${.CURDIR}/../display/modules/hdcp \
	${.CURDIR}/../display/modules/info_packet \
	${.CURDIR}/../display/modules/power \
	${.CURDIR}/../display/modules/vmid \
	${.CURDIR}/../pm/swsmu \
	${.CURDIR}/../pm/swsmu/smu11 \
	${.CURDIR}/../pm/swsmu/smu12 \
	${.CURDIR}/../pm/swsmu/smu13 \
	${.CURDIR}/../amdkfd

SRCS=	amdgpu_drv.c

SRCS+=	chash.c

# add KMS driver
SRCS+= amdgpu_device.c amdgpu_kms.c \
	amdgpu_atombios.c amdgpu_atombios_crtc.c amdgpu_connectors.c \
	amdgpu_atom.c amdgpu_fence.c amdgpu_ttm.c amdgpu_object.c amdgpu_gart.c \
	amdgpu_encoders.c amdgpu_display.c amdgpu_i2c.c \
	amdgpu_fb.c amdgpu_gem.c amdgpu_ring.c \
	amdgpu_cs.c amdgpu_bios.c amdgpu_benchmark.c amdgpu_test.c \
	amdgpu_atombios_dp.c amdgpu_afmt.c amdgpu_trace_points.c \
	amdgpu_atombios_encoders.c amdgpu_sa.c amdgpu_atombios_i2c.c \
	amdgpu_prime.c amdgpu_vm.c amdgpu_ib.c amdgpu_pll.c \
	amdgpu_ucode.c amdgpu_bo_list.c amdgpu_ctx.c amdgpu_sync.c \
	amdgpu_gtt_mgr.c amdgpu_vram_mgr.c amdgpu_virt.c amdgpu_atomfirmware.c \
	amdgpu_queue_mgr.c amdgpu_vf_error.c amdgpu_sched.c amdgpu_debugfs.c \
	amdgpu_ids.c

SRCS+= \
	vi.c mxgpu_vi.c nbio_v6_1.c soc15.c emu_soc.c mxgpu_ai.c nbio_v7_0.c vega10_reg_init.c \
	vega20_reg_init.c

# add DF block
SRCS+= \
	df_v1_7.c \
	df_v3_6.c

# add GMC block
SRCS+= \
	gmc_v7_0.c \
	gmc_v8_0.c \
	gfxhub_v1_0.c mmhub_v1_0.c gmc_v9_0.c

# add IH block
SRCS+= \
	amdgpu_irq.c \
	amdgpu_ih.c \
	iceland_ih.c \
	tonga_ih.c \
	cz_ih.c \
	vega10_ih.c

# add PSP block
SRCS+= \
	amdgpu_psp.c \
	psp_v3_1.c \
	psp_v10_0.c

# add SMC block
SRCS+= \
	amdgpu_dpm.c

# dce_virtual.c replaced in 5.15 by shortened amdgpu_vkms.c

# add DCE block
SRCS+= \
	dce_v10_0.c \
	dce_v11_0.c \
	dce_virtual.c

# add GFX block
SRCS+= \
	amdgpu_gfx.c \
	gfx_v8_0.c \
	gfx_v9_0.c

# add async DMA block
SRCS+= \
	sdma_v2_4.c \
	sdma_v3_0.c \
	sdma_v4_0.c

# add UVD block
SRCS+= \
	amdgpu_uvd.c \
	uvd_v5_0.c \
	uvd_v6_0.c \
	uvd_v7_0.c

# add VCE block
SRCS+= \
	amdgpu_vce.c \
	vce_v3_0.c \
	vce_v4_0.c

# add VCN block
SRCS+= \
	amdgpu_vcn.c \
	vcn_v1_0.c

# add amdkfd interfaces
SRCS+= amdgpu_amdkfd.c

# add cgs
SRCS+= amdgpu_cgs.c

# GPU scheduler
SRCS+= \
	sched_main.c \
	sched_fence.c \
	sched_entity.c \
	amdgpu_job.c

SRCS+= amdgpu_acpi.c

SRCS+= amd_powerplay.c

# powerplay/hwmgr
SRCS+= hwmgr.c processpptables.c \
		hardwaremanager.c smu8_hwmgr.c \
		pppcielanes.c\
		process_pptables_v1_0.c ppatomctrl.c ppatomfwctrl.c \
		smu7_hwmgr.c smu7_powertune.c smu7_thermal.c \
		smu7_clockpowergating.c \
		vega10_processpptables.c vega10_hwmgr.c vega10_powertune.c \
		vega10_thermal.c smu10_hwmgr.c pp_psm.c\
		vega12_processpptables.c vega12_hwmgr.c \
		vega12_thermal.c \
		pp_overdriver.c smu_helper.c

# powerplay/smumgr
SRCS+= smumgr.c smu8_smumgr.c tonga_smumgr.c fiji_smumgr.c \
	  polaris10_smumgr.c iceland_smumgr.c \
	  smu7_smumgr.c vega10_smumgr.c smu10_smumgr.c ci_smumgr.c \
	  vega12_smumgr.c vegam_smumgr.c smu9_smumgr.c \
	  vega20_smumgr.c

# display core component (CONFIG_DRM_AMD_DC)
# amdgpu_dm
SRC+= amdgpu_dm.c amdgpu_dm_irq.c amdgpu_dm_mst_types.c amdgpu_dm_color.c
# ifneq ($(CONFIG_DRM_AMD_DC),)
SRC+= amdgpu_dm_services.c amdgpu_dm_helpers.c amdgpu_dm_pp_smu.c

# dc
SRCS+= amdgpu_dm.c amdgpu_dm_irq.c amdgpu_dm_mst_types.c amdgpu_dm_color.c
SRCS+= amdgpu_dm_services.c amdgpu_dm_helpers.c amdgpu_dm_pp_smu.c

# dc/basics
SRCS+= conversion.c fixpt31_32.c \
	log_helpers.c vector.c

# dc/bios
SRCS+= bios_parser.c bios_parser_interface.c  bios_parser_helper.c \
	command_table.c command_table_helper.c bios_parser_common.c
SRCS+= command_table2.c command_table_helper2.c bios_parser2.c
# DCE 8x
# All DCE8.x are derived from DCE8.0, so 8.0 MUST be defined if ANY of
# DCE8.x is compiled.
SRCS+= command_table_helper_dce80.c

# DCE 11x
SRCS+= command_table_helper_dce110.c
SRCS+= command_table_helper_dce112.c
SRCS+= command_table_helper2_dce112.c

# dc/calcs
SRCS+= dce_calcs.c bw_fixed.c custom_float.c

# ifdef CONFIG_DRM_AMD_DC_DCN1_0
SRCS+= dcn_calcs.c dcn_calc_math.c dcn_calc_auto.c

# dc/dce
SRCS+= dce_audio.c dce_stream_encoder.c dce_link_encoder.c dce_hwseq.c \
	dce_mem_input.c dce_clock_source.c dce_scl_filters.c dce_transform.c \
	dce_clocks.c dce_opp.c dce_dmcu.c dce_abm.c dce_ipp.c dce_aux.c

# dc/gpio
SRCS+= gpio_base.c gpio_service.c hw_factory.c \
       hw_gpio.c hw_hpd.c hw_ddc.c hw_translate.c

# DCE 8x
# all DCE8.x are derived from DCE8.0
SRCS+= hw_translate_dce80.c hw_factory_dce80.c

# DCE 11x
SRCS+= hw_translate_dce110.c hw_factory_dce110.c

# DCE 12x
SRCS+= hw_translate_dce120.c hw_factory_dce120.c

# DCN 1x
# ifdef CONFIG_DRM_AMD_DC_DCN1_0
SRCS+= hw_translate_dcn10.c hw_factory_dcn10.c

# Diagnostics on FPGA
SRCS+= hw_translate_diag.c hw_factory_diag.c

# dc/i2caux
SRCS+= aux_engine.c engine_base.c i2caux.c i2c_engine.c \
	i2c_generic_hw_engine.c i2c_hw_engine.c i2c_sw_engine.c
# DCE 8x family
SRCS+= i2caux_dce80.c i2c_hw_engine_dce80.c \
	i2c_sw_engine_dce80.c

# DCE 100 family
SRCS+= i2caux_dce100.c

# DCE 110 family
SRCS+= i2caux_dce110.c i2c_sw_engine_dce110.c i2c_hw_engine_dce110.c \
	aux_engine_dce110.c

# DCE 112 family
SRCS+= i2caux_dce112.c

# DCN 1.0 family
# ifdef CONFIG_DRM_AMD_DC_DCN1_0
SRCS+= i2caux_dcn10.c

# DCE 120 family
SRCS+= i2caux_dce120.c

SRCS+= i2caux_diag.c

# dc/irq
SRCS+= irq_service.c

# DCE 8x
SRCS+= irq_service_dce80.c

# DCE 11x
SRCS+= irq_service_dce110.c

# DCE 12x
SRCS+= irq_service_dce120.c

# DCN 1x
# ifdef CONFIG_DRM_AMD_DC_DCN1_0
SRCS+= irq_service_dcn10.c

# dc/virtual
SRCS+= virtual_link_encoder.c virtual_stream_encoder.c

# dc/dcn10
SRCS+= dcn10_resource.c dcn10_ipp.c dcn10_hw_sequencer.c \
	dcn10_dpp.c dcn10_opp.c dcn10_optc.c \
	dcn10_hubp.c dcn10_mpc.c \
	dcn10_dpp_dscl.c dcn10_dpp_cm.c dcn10_cm_common.c \
	dcn10_hubbub.c dcn10_stream_encoder.c dcn10_link_encoder.c

# dc/dml
SRCS+= display_mode_lib.c display_rq_dlg_helpers.c dml1_display_rq_dlg_calc.c \
	dml_common_defs.c

# dc/dce120
SRCS+= dce120_resource.c dce120_timing_generator.c \
	dce120_hw_sequencer.c

# dc/dce112
SRCS+= dce112_compressor.c dce112_hw_sequencer.c \
	dce112_resource.c

# dc/dce110
SRCS+= dce110_timing_generator.c \
	dce110_compressor.c dce110_hw_sequencer.c dce110_resource.c \
	dce110_opp_regamma_v.c dce110_opp_csc_v.c dce110_timing_generator_v.c \
	dce110_mem_input_v.c dce110_opp_v.c dce110_transform_v.c

# dc/dce100
SRCS+= dce100_resource.c dce100_hw_sequencer.c

# dc/dce80
SRCS+= dce80_timing_generator.c dce80_hw_sequencer.c \
	dce80_resource.c

SRCS+= amdgpu_dc.c dc_link.c dc_resource.c dc_hw_sequencer.c dc_sink.c \
	dc_surface.c dc_link_hwss.c dc_link_dp.c dc_link_ddc.c dc_debug.c dc_stream.c
SRCS+= dc_helper.c

# modules/freesync
SRCS+= freesync.c

# modules/color
SRCS+= color_gamma.c

# end display core component (CONFIG_DRM_AMD_DC)

# amdgpu_prime.c refactored into amdgpu_dma_buf.c

# Added not enabled from openbsd drm 5.15
SRCS+= \
	aldebaran_reg_init.c \
	amdgpu_amdkfd_aldebaran.c \
	amdgpu_amdkfd_arcturus.c \
	amdgpu_amdkfd_fence.c \
	amdgpu_amdkfd_gfx_v10.c \
	amdgpu_amdkfd_gfx_v10_3.c \
	amdgpu_amdkfd_gfx_v7.c \
	amdgpu_amdkfd_gpuvm.c \
	amdgpu_cik_sdma.c \
	amdgpu_csa.c \
	amdgpu_discovery.c \
	amdgpu_dma_buf.c \
	amdgpu_eeprom.c \
	amdgpu_fdinfo.c \
	amdgpu_fru_eeprom.c \
	amdgpu_fw_attestation.c \
	amdgpu_gmc.c \
	amdgpu_hdp.c \
	amdgpu_ioc32.c \
        amdgpu_jpeg.c \
	amdgpu_mca.c \
	amdgpu_mmhub.c \
	amdgpu_nbio.c \
	amdgpu_pmu.c \
	amdgpu_preempt_mgr.c \
	amdgpu_rap.c \
	amdgpu_ras.c \
	amdgpu_ras_eeprom.c \
	amdgpu_reset.c \
	amdgpu_rlc.c \
	amdgpu_sdma.c \
	amdgpu_securedisplay.c \
	amdgpu_si.c \
	amdgpu_si_dma.c \
	amdgpu_umc.c \
	amdgpu_uvd_v3_1.c \
	amdgpu_uvd_v4_2.c \
	amdgpu_vce_v2_0.c \
	amdgpu_vkms.c \
	amdgpu_vm_cpu.c \
	amdgpu_vm_sdma.c \
	amdgpu_xgmi.c \
	arct_reg_init.c \
	athub_v1_0.c \
	athub_v2_0.c \
	athub_v2_1.c \
	beige_goby_reg_init.c \
	cik_ih.c \
	cyan_skillfish_reg_init.c \
	dce_v6_0.c \
	dce_v8_0.c \
	dimgrey_cavefish_reg_init.c \
	gfx_v10_0.c \
	gfx_v6_0.c \
	gfx_v7_0.c \
	gfx_v9_4.c \
	gfx_v9_4_2.c \
	gfxhub_v1_1.c \
	gfxhub_v2_0.c \
	gfxhub_v2_1.c \
	gmc_v10_0.c \
	gmc_v6_0.c \
	hdp_v4_0.c \
	hdp_v5_0.c \
	jpeg_v1_0.c \
	jpeg_v2_0.c \
	jpeg_v2_5.c \
	jpeg_v3_0.c \
	mca_v3_0.c \
	mes_v10_1.c \
	mmhub_v1_7.c \
	mmhub_v2_0.c \
	mmhub_v2_3.c \
	mmhub_v9_4.c \
	mxgpu_nv.c \
	navi10_ih.c \
	navi10_reg_init.c \
	navi12_reg_init.c \
	navi14_reg_init.c \
	nbio_v2_3.c \
	nbio_v7_2.c \
	nbio_v7_4.c \
	nv.c \
	psp_v11_0.c \
	psp_v11_0_8.c \
	psp_v12_0.c \
	psp_v13_0.c \
	sdma_v4_4.c \
	sdma_v5_0.c \
	sdma_v5_2.c \
	si_ih.c \
	sienna_cichlid_reg_init.c \
	smu_v11_0_i2c.c \
	smuio_v11_0.c \
	smuio_v11_0_6.c \
	smuio_v13_0.c \
	smuio_v9_0.c \
	umc_v6_0.c \
	umc_v6_1.c \
	umc_v6_7.c \
	umc_v8_7.c \
	vangogh_reg_init.c \
	vcn_v2_0.c \
	vcn_v2_5.c \
	vcn_v3_0.c \
	vega20_ih.c \
	yellow_carp_reg_init.c \
	aldebaran.c

# amdkfd
SRCS+= \
	cik_event_interrupt.c \
	kfd_chardev.c \
	kfd_crat.c \
	kfd_dbgdev.c \
	kfd_dbgmgr.c \
	kfd_debugfs.c \
	kfd_device.c \
	kfd_device_queue_manager.c \
	kfd_device_queue_manager_cik.c \
	kfd_device_queue_manager_v10.c \
	kfd_device_queue_manager_v9.c \
	kfd_device_queue_manager_vi.c \
	kfd_doorbell.c \
	kfd_events.c \
	kfd_flat_memory.c \
	kfd_int_process_v9.c \
	kfd_interrupt.c \
	kfd_iommu.c \
	kfd_kernel_queue.c \
	kfd_migrate.c \
	kfd_module.c \
	kfd_mqd_manager.c \
	kfd_mqd_manager_cik.c \
	kfd_mqd_manager_v10.c \
	kfd_mqd_manager_v9.c \
	kfd_mqd_manager_vi.c \
	kfd_packet_manager.c \
	kfd_packet_manager_v9.c \
	kfd_packet_manager_vi.c \
	kfd_pasid.c \
	kfd_process.c \
	kfd_process_queue_manager.c \
	kfd_queue.c \
	kfd_smi_events.c \
	kfd_svm.c \
	kfd_topology.c

# display/amdgpu_dm new files from openbsd
SRCS+= \
	amdgpu_dm_psr.c \
	dc_fpu.c \
	amdgpu_dm_hdcp.c

# display/dc new files from openbsd
SRCS+= \
	dc_dmub_srv.c

# display/dc/basics new files from openbsd
SRCS+= \
	dc_common.c \
	amdgpu_vector.c \
	dc_edid_parser.c \
	dce_clk_mgr.c

# display/dc/bios new files from openbsd
SRCS+= \
	command_table_helper_dce60.c	

# cik.c moved to amdgpu_cik.c

# display/dc/clk_mgr new files from openbsd
SRCS+= \
	clk_mgr.c \
	dce_clk_mgr.c \
	dce110_clk_mgr.c \
	dce112_clk_mgr.c \
	dce120_clk_mgr.c \
	dce60_clk_mgr.c \
	rv1_clk_mgr.c \
	rv1_clk_mgr_clk.c \
	rv1_clk_mgr_vbios_smu.c \
	rv2_clk_mgr.c \
	dcn20_clk_mgr.c \
	rn_clk_mgr.c \
	rn_clk_mgr_vbios_smu.c \
	dcn30_clk_mgr.c \
	dcn30_clk_mgr_smu_msg.c \
	dcn301_smu.c \
	vg_clk_mgr.c \
	dcn31_clk_mgr.c \
	dcn31_smu.c

# display/dc/core new files from openbsd
SRCS+= \
	dc_link_dpcd.c \
	dc_link_enc_cfg.c \
	dc_stat.c \
	dc_vm_helper.c

# display/dc/dce new files from openbsd
SRCS+= \
	dce_i2c.c \
	dce_i2c_hw.c \
	dce_i2c_sw.c \
	dce_panel_cntl.c \
	dce_scl_filters_old.c \
	dmub_abm.c \
	dmub_hw_lock_mgr.c \
	dmub_outbox.c \
	dmub_psr.c

# display/dc/dce60 new files from openbsd
SRCS+= \
	dce60_hw_sequencer.c \
	dce60_resource.c \
	dce60_timing_generator.c

# display/dc/dcn20 new files from openbsd
SRCS+= \
	dcn20_dccg.c \
	dcn20_dpp.c \
	dcn20_dpp_cm.c\
	dcn20_dsc.c \
	dcn20_dwb.c \
	dcn20_dwb_scl.c \
	dcn20_hubbub.c \
	dcn20_hubp.c \
	dcn20_hwseq.c \
	dcn20_init.c \
	dcn20_link_encoder.c \
	dcn20_mmhubbub.c \
	dcn20_mpc.c \
	dcn20_opp.c \
	dcn20_optc.c \
	dcn20_resource.c \
	dcn20_stream_encoder.c \
	dcn20_vmid.c

# display/dc/dcn21 new files from openbsd
SRCS+= \
	dcn21_dccg.c \
	dcn21_hubbub.c \
	dcn21_hubp.c \
	dcn21_hwseq.c \
	dcn21_init.c \
	dcn21_link_encoder.c \
	dcn21_resource.c

# display/dc/dcn30 new files from openbsd
SRCS+= \
	dcn30_afmt.c \
	dcn30_cm_common.c \
	dcn30_dccg.c \
	dcn30_dio_link_encoder.c \
	dcn30_dio_stream_encoder.c \
	dcn30_dpp.c \
	dcn30_dpp_cm.c\
	dcn30_dwb.c \
	dcn30_dwb_cm.c \
	dcn30_hubbub.c \
	dcn30_hubp.c \
	dcn30_hwseq.c \
	dcn30_init.c \
	dcn30_mmhubbub.c \
	dcn30_mpc.c \
	dcn30_optc.c \
	dcn30_resource.c \
	dcn30_vpg.c

# display/dc/dcn301 new files from openbsd
SRCS+= \
	dcn301_dccg.c \
	dcn301_dio_link_encoder.c \
	dcn301_hubbub.c \
	dcn301_hwseq.c \
	dcn301_init.c \
	dcn301_panel_cntl.c \
	dcn301_resource.c

# display/dc/dcn302 new files from openbsd
SRCS+= \
	dcn302_hwseq.c \
	dcn302_init.c \
	dcn302_resource.c

# display/dc/dcn303 new files from openbsd
SRCS+= \
	dcn303_hwseq.c \
	dcn303_init.c \
	dcn303_resource.c

# display/dc/dcn31 new files from openbsd
SRCS+= \
	dcn31_dccg.c \
	dcn31_dio_link_encoder.c \
	dcn31_hubbub.c \
	dcn31_hubp.c \
	dcn31_hwseq.c \
	dcn31_init.c \
	dcn31_optc.c \
	dcn31_panel_cntl.c \
	dcn31_resource.c

# display/dc/dml new files from openbsd
SRCS+= \
	display_mode_vba.c \
	display_mode_vba_20.c \
	display_mode_vba_20v2.c \
	display_rq_dlg_calc_20.c \
	display_rq_dlg_calc_20v2.c \
	display_mode_vba_21.c \
	display_rq_dlg_calc_21.c \
	dcn2x.c \
	display_mode_vba_30.c \
	display_rq_dlg_calc_30.c \
	display_mode_vba_31.c \
	display_rq_dlg_calc_31.c

# display/dc/dcn10 new files from openbsd
SRCS+= \
	dcn10_dwb.c \
	dcn10_hw_sequencer_debug.c \
	dcn10_init.c

# display/dc/dml/dsc new files from openbsd
SRCS+= \
	rc_calc_fpu.c

# display/dc/dsc new files from openbsd
SRCS+= \
	dc_dsc.c \
	rc_calc.c \
	rc_calc_dpi.c

# display/dc/gpio new files from openbsd
SRCS+= \
	hw_generic.c

# display/dc/gpio/dce60 new files from openbsd
SRCS+= \
	hw_factory_dce60.c \
	hw_translate_dce60.c

# display/dc/gpio/dcn* new files from openbsd
SRCS+= \
	hw_factory_dcn20.c \
	hw_translate_dcn20.c \
	hw_factory_dcn21.c \
	hw_translate_dcn21.c \
	hw_factory_dcn30.c \
	hw_translate_dcn30.c

# display/dc/hdcp new files from openbsd
SRCS+= \
	hdcp_msg.c

# display/dc/irq new files from openbsd
SRCS+= \
	irq_service_dce60.c

# display/dmub/src new files from openbsd
SRCS+= \
	dmub_dcn20.c \
	dmub_dcn21.c \
	dmub_dcn30.c \
	dmub_dcn301.c \
	dmub_dcn302.c \
	dmub_dcn303.c \
	dmub_dcn31.c \
	dmub_reg.c \
	dmub_srv.c \
	dmub_srv_stat.c

# display/modules new files from openbsd
SRCS+= \
	color_table.c \
	hdcp.c \
	hdcp1_execution.c \
	hdcp1_transition.c \
	hdcp2_execution.c \
	hdcp2_transition.c \
	hdcp_ddc.c \
	hdcp_log.c \
	hdcp_psp.c

# pm new and rearranged files from openbsd
SRCS+= \
	amdgpu_pm.c

# pm/powerplay new and rearranged files from openbsd
SRCS+= \
	amdgpu_kv_dpm.c \
	amdgpu_kv_smc.c \
	amdgpu_si_smc.c \
	ci_baco.c \
	common_baco.c \
	fiji_baco.c \
	polaris_baco.c \
	smu7_baco.c \
	smu9_baco.c \
	tonga_baco.c \
	vega10_baco.c \
	vega12_baco.c \
	vega20_baco.c  \
	vega20_hwmgr.c \
	vega20_powertune.c \
	vega20_processpptables.c \
	vega20_thermal.c

# pm/swsmu new files from openbsd
SRCS+= \
	amdgpu_smu.c \
	smu_cmn.c

# pm/swsmu/smu11 new files from openbsd
SRCS+= \
	arcturus_ppt.c \
	cyan_skillfish_ppt.c \
	navi10_ppt.c \
	sienna_cichlid_ppt.c \
	smu_v11_0.c \
	vangogh_ppt.c

# pm/swsmu/smu12 new files from openbsd
SRCS+= \
	renoir_ppt.c \
	smu_v12_0.c

# pm/swsmu/smu13 new files from openbsd
SRCS+= \
	aldebaran_ppt.c \
	smu_v13_0.c \
	yellow_carp_ppt.c

SRCS+= \
	opt_ddb.h 	\
	opt_acpi.h	\
	opt_drm.h 	\
	acpi_if.h	\
	bus_if.h	\
	device_if.h	\
	pci_if.h

# From linux
#TODO: remove when Timing Sync feature is complete
KCFLAGS+= -DBUILD_FEATURE_TIMING_SYNC=0

KCFLAGS+= -I${SYSDIR}/dev/drm/include
KCFLAGS+= -I${SYSDIR}/dev/drm/include/drm
KCFLAGS+= -I${SYSDIR}/dev/drm/include/uapi
KCFLAGS+= -I${SYSDIR}/dev/drm/include/uapi/drm
KCFLAGS+= -I${SYSDIR}/contrib/dev/acpica/source/include
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/amdgpu
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/include
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/dc
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/dc/inc
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/dc/inc/hw
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/modules/inc
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/modules/freesync
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/modules/color
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/amdgpu_dm
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/include
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/dmub
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/display/dmub/inc
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/include
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/include/asic_reg
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/pm/inc
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/pm/powerplay
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/pm/powerplay/hwmgr
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/pm/powerplay/smumgr
KCFLAGS+= -I${SYSDIR}/dev/drm/amd/pm/swsmu
KCFLAGS+= -I${SYSDIR}/dev/drm/scheduler
KCFLAGS+= -include ${SYSDIR}/dev/drm/kconfig.h

.include <bsd.kmod.mk>

dcn_calcs.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}

dcn_calc_auto.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}

dcn_calc_math.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}

dcn20_resource.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}

dcn21_resource.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}

dml1_display_rq_dlg_calc.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}

dml_common_defs.o:
	${CC} ${CFLAGS} ${KCFLAGS} -mhard-float -msse -mpreferred-stack-boundary=4 -c ${.IMPSRC}
